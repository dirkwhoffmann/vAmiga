// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include <metal_stdlib>

using namespace metal;

#define SCALE_FACTOR 2


//
// Vertex shader data types
// 

struct InVertex {
    float4 position [[attribute(0)]];
    float2 texCoords [[attribute(1)]];
};

struct ProjectedVertex {
    float4 position [[position]];
    float2 texCoords [[user(tex_coords)]];
};

struct Uniforms {
    float4x4 modelViewProjection;
};


//
// Compute shader data types
//

struct ShaderOptions {
    
    uint blur;
    float blurRadius;

    uint bloom;
    float bloomRadiusR;
    float bloomRadiusG;
    float bloomRadiusB;
    float bloomBrightness;
    float bloomWeight;

    uint dotMask;
    float dotMaskBrightness;

    uint scanlines;
    float scanlineBrightness;
    float scanlineWeight;
    
    uint disalignment;
    float disalignmentH;
    float disalignmentV;
};


//
// Fragment shader data types
//

struct FragmentUniforms {
    
    float alpha;
    uint dotMaskWidth;
    uint dotMaskHeight;
    uint scanlineDistance;
};


//
// Vertex shader
//

vertex ProjectedVertex vertex_main(device InVertex *vertices [[buffer(0)]],
                                   constant Uniforms &uniforms [[buffer(1)]],
                                   ushort vid [[vertex_id]])
{
    ProjectedVertex out;

    out.position = uniforms.modelViewProjection * float4(vertices[vid].position);
    out.texCoords = vertices[vid].texCoords;
    return out;
}


//
// Fragment shader
//

float4 scanlineWeight(uint2 pixel, uint height, float weight, float brightness, float bloom) {
    
    // Calculate distance to nearest scanline
    float dy = ((float(pixel.y % height) / float(height - 1)) - 0.5);
 
    // Calculate scanline weight
    float scanlineWeight = max(1.0 - dy * dy * 24 * weight, brightness);
    
    // Apply bloom effect an return
    return scanlineWeight * bloom;
}

float3 rgb2hsv(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = mix(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = mix(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float3 hsv2rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

fragment half4 fragment_main(ProjectedVertex vert [[ stage_in ]],
                             texture2d<float, access::sample> texture [[ texture(0) ]],
                             texture2d<float, access::sample> bloomTextureR [[ texture(1) ]],
                             texture2d<float, access::sample> bloomTextureG [[ texture(2) ]],
                             texture2d<float, access::sample> bloomTextureB [[ texture(3) ]],
                             texture2d<float, access::sample> dotMask [[ texture(4) ]],
                             constant ShaderOptions &options [[ buffer(0) ]],
                             constant FragmentUniforms &uniforms [[ buffer(1) ]],
                             sampler texSampler [[ sampler(0) ]])
{
    uint x = uint(vert.position.x);
    uint y = uint(vert.position.y);
    uint2 pixel = uint2(x, y);
    float4 color;
    
    // Read fragment from texture
    float2 tc = float2(vert.texCoords.x, vert.texCoords.y);
    if (options.disalignment) {
        float dx = options.disalignmentH;
        float dy = options.disalignmentV;
        float4 r = texture.sample(texSampler, tc + float2(dx,dy));
        float4 g = texture.sample(texSampler, tc);
        float4 b = texture.sample(texSampler, tc - float2(dx,dy));
        color = float4(r.r, g.g, b.b,0);
    } else {
        color = texture.sample(texSampler, float2(vert.texCoords.x, vert.texCoords.y));
    }

    // float luma = (0.2126 * color.r) + (0.7152 * color.g) + (0.0722 * color.b);
    
    // Apply bloom effect (if enabled)
    if (options.bloom) {
        float4 bloom_r = bloomTextureR.sample(texSampler, tc);
        float4 bloom_g = bloomTextureG.sample(texSampler, tc);
        float4 bloom_b = bloomTextureB.sample(texSampler, tc);
        float4 bColor = bloom_r + bloom_g + bloom_b;
        bColor = pow(bColor, options.bloomWeight) * options.bloomBrightness;
        // color = bColor;
        color = saturate(color + bColor);
    }
    
    // Apply scanline effect (if emulation type matches)
    if (options.scanlines == 2) {
        color *= scanlineWeight(pixel,
                                uniforms.scanlineDistance,
                                options.scanlineWeight,
                                options.scanlineBrightness,
                                1.0);
    }

    if (options.dotMask) {
        
        uint xoffset = x % uniforms.dotMaskWidth;
        uint yoffset = y % uniforms.dotMaskHeight;
        float4 dotColor = dotMask.read(uint2(xoffset, yoffset));
        float4 gain = min(color, 1 - color) * dotColor;
        float4 loose = min(color, 1 - color) * 0.5 * (1 - dotColor);
        color += gain - loose;
    }
    
    return half4(color.r, color.g, color.b, uniforms.alpha);
}


//
// Texture merger
//

kernel void merge(texture2d<half, access::read>  longFrame   [[ texture(0) ]],
                  texture2d<half, access::read>  shortFrame  [[ texture(1) ]],
                  texture2d<half, access::write> outTexture  [[ texture(2) ]],
                  uint2                          gid         [[ thread_position_in_grid ]])
{
    half4 result;
    
    if (gid.y % 2 == 0) {
        result = longFrame.read(uint2(gid.x, gid.y / 2));
    } else {
        result = shortFrame.read(uint2(gid.x, gid.y / 2));
    }
    
    outTexture.write(result, gid);
}


//
// Texture upscalers
//

kernel void bypassupscaler(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                           texture2d<half, access::write> outTexture  [[ texture(1) ]],
                           uint2                          gid         [[ thread_position_in_grid ]])
{
    half4 result = inTexture.read(uint2(gid.x / SCALE_FACTOR, gid.y / SCALE_FACTOR));
    outTexture.write(result, gid);
}

// EPX upscaler (Eric's Pixel Expansion)
void writePixelBlock(texture2d<half, access::write> outTexture, uint2 gid, half4 value)
{
    outTexture.write(value, gid + uint2(0,0));
    outTexture.write(value, gid + uint2(0,1));
    outTexture.write(value, gid + uint2(1,0));
    outTexture.write(value, gid + uint2(1,1));
}

kernel void epxupscaler(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                        texture2d<half, access::write> outTexture  [[ texture(1) ]],
                        uint2                          gid         [[ thread_position_in_grid ]])
{
    if((gid.x % SCALE_FACTOR != 0) || (gid.y % SCALE_FACTOR != 0))
        return;
    
    //   A    --\ 1 2
    // C P B  --/ 3 4
    //   D
    // 1=P; 2=P; 3=P; 4=P;
    // IF C==A AND C!=D AND A!=B => 1=A
    // IF A==B AND A!=C AND B!=D => 2=B
    // IF D==C AND D!=B AND C!=A => 3=C
    // IF B==D AND B!=A AND D!=C => 4=D
    
    half xx = gid.x / SCALE_FACTOR;
    half yy = gid.y / SCALE_FACTOR;
    half4 A = inTexture.read(uint2(xx, yy - 1));
    half4 C = inTexture.read(uint2(xx - 1, yy));
    half4 P = inTexture.read(uint2(xx, yy));
    half4 B = inTexture.read(uint2(xx + 1, yy));
    half4 D = inTexture.read(uint2(xx, yy + 1));
    
    half4 r1 = (all(C == A) && any(C != D) && any(A != B)) ? A : P;
    half4 r2 = (all(A == B) && any(A != C) && any(B != D)) ? B : P;
    half4 r3 = (all(A == B) && any(A != C) && any(B != D)) ? C : P;
    half4 r4 = (all(B == D) && any(B != A) && any(D != C)) ? D : P;
    
    outTexture.write(r1, gid + uint2(0,0));
    outTexture.write(r2, gid + uint2(1,0));
    outTexture.write(r3, gid + uint2(0,1));
    outTexture.write(r4, gid + uint2(1,1));

    /*
    writePixelBlock(outTexture, gid + uint2(0,0), r1);
    writePixelBlock(outTexture, gid + uint2(2,0), r2);
    writePixelBlock(outTexture, gid + uint2(0,2), r3);
    writePixelBlock(outTexture, gid + uint2(2,2), r4);
    */
}


#if 0
// xBR upscaler (4x)
// Code is based on what I've found at:
// https://gamedev.stackexchange.com/questions/87275/how-do-i-perform-an-xbr-or-hqx-filter-in-xna

constant half coef = 2.0;
constant half3 yuv_weighted = half3(14.352, 28.176, 5.472);

half4 df(half4 A, half4 B)
{
    return abs(A - B);
}

half4 weighted_distance(half4 a, half4 b, half4 c, half4 d,
                        half4 e, half4 f, half4 g, half4 h)
{
    return (df(a, b) + df(a, c) + df(d, e) + df(d, f) + 4.0 * df(g, h));
}

kernel void xbrupscaler(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                        texture2d<half, access::write> outTexture  [[ texture(1) ]],
                        uint2                          gid         [[ thread_position_in_grid ]])
{
    bool4 edr, edr_left, edr_up, px;        // px = pixel, edr = edge detection rule
    bool4 ir_lv1, ir_lv2_left, ir_lv2_up;
    bool4 nc;                               // new color
    bool4 fx, fx_left, fx_up;               // inequations of straight lines
        
    half2 fp = fract(half2(gid) / SCALE_FACTOR);
    half2 ggid = half2(gid) / SCALE_FACTOR; // half2(gid) / SCALE_FACTOR;
    
    half2 dx = half2(1.0, 0.0);
    half2 dy = half2(0.0, 1.0);
    
    /*
    half3 A  = inTexture.read(ggid +  uint2(-1,-1)).xyz;
    half3 B  = inTexture.read(ggid + uint2( 0,-1)).xyz;
    half3 C  = inTexture.read(ggid + uint2( 1,-1)).xyz;
    half3 D  = inTexture.read(ggid + uint2(-1, 0)).xyz;
    half3 E  = inTexture.read(ggid + uint2( 0, 0)).xyz;
    half3 F  = inTexture.read(ggid + uint2( 1, 0)).xyz;
    half3 G  = inTexture.read(ggid + uint2(-1, 1)).xyz;
    half3 H  = inTexture.read(ggid + uint2( 0, 1)).xyz;
    half3 I  = inTexture.read(ggid + uint2( 1, 1)).xyz;
    half3 A1 = inTexture.read(ggid + uint2(-1,-2)).xyz;
    half3 C1 = inTexture.read(ggid + uint2( 1,-2)).xyz;
    half3 A0 = inTexture.read(ggid + uint2(-2,-1)).xyz;
    half3 G0 = inTexture.read(ggid + uint2(-2, 1)).xyz;
    half3 C4 = inTexture.read(ggid + uint2( 2,-1)).xyz;
    half3 I4 = inTexture.read(ggid + uint2( 2, 1)).xyz;
    half3 G5 = inTexture.read(ggid + uint2(-1, 2)).xyz;
    half3 I5 = inTexture.read(ggid + uint2( 1, 2)).xyz;
    half3 B1 = inTexture.read(ggid + uint2( 0,-2)).xyz;
    half3 D0 = inTexture.read(ggid + uint2(-2, 0)).xyz;
    half3 H5 = inTexture.read(ggid + uint2( 0, 2)).xyz;
    half3 F4 = inTexture.read(ggid + uint2( 2, 0)).xyz;
    */
    half3 A  = inTexture.read(uint2(ggid   - dx   - dy)).xyz;
    half3 B  = inTexture.read(uint2(ggid          - dy)).xyz;
    half3 C  = inTexture.read(uint2(ggid   + dx   - dy)).xyz;
    half3 D  = inTexture.read(uint2(ggid   - dx       )).xyz;
    half3 E  = inTexture.read(uint2(ggid              )).xyz;
    half3 F  = inTexture.read(uint2(ggid   + dx       )).xyz;
    half3 G  = inTexture.read(uint2(ggid   - dx   + dy)).xyz;
    half3 H  = inTexture.read(uint2(ggid          + dy)).xyz;
    half3 I  = inTexture.read(uint2(ggid   + dx   + dy)).xyz;
    half3 A1 = inTexture.read(uint2(ggid   - dx - 2*dy)).xyz;
    half3 C1 = inTexture.read(uint2(ggid   + dx - 2*dy)).xyz;
    half3 A0 = inTexture.read(uint2(ggid - 2*dx   - dy)).xyz;
    half3 G0 = inTexture.read(uint2(ggid - 2*dx   + dy)).xyz;
    half3 C4 = inTexture.read(uint2(ggid + 2*dx   - dy)).xyz;
    half3 I4 = inTexture.read(uint2(ggid + 2*dx   + dy)).xyz;
    half3 G5 = inTexture.read(uint2(ggid   - dx + 2*dy)).xyz;
    half3 I5 = inTexture.read(uint2(ggid   + dx + 2*dy)).xyz;
    half3 B1 = inTexture.read(uint2(ggid        - 2*dy)).xyz;
    half3 D0 = inTexture.read(uint2(ggid - 2*dx       )).xyz;
    half3 H5 = inTexture.read(uint2(ggid        + 2*dy)).xyz;
    half3 F4 = inTexture.read(uint2(ggid + 2*dx       )).xyz;
    
    half4 b = yuv_weighted * half4x3(B, D, H, F);
    half4 c = yuv_weighted * half4x3(C, A, G, I);
    half4 e = yuv_weighted * half4x3(E, E, E, E);
    half4 d = b.yzwx;
    half4 f = b.wxyz;
    half4 g = c.zwxy;
    half4 h = b.zwxy;
    half4 i = c.wxyz;
        
    half4 i4 = yuv_weighted * half4x3(I4, C1, A0, G5);
    half4 i5 = yuv_weighted * half4x3(I5, C4, A1, G0);
    half4 h5 = yuv_weighted * half4x3(H5, F4, B1, D0);
    half4 f4 = h5.yzwx;
        
    half4 Ao = half4(1.0, -1.0, -1.0, 1.0);
    half4 Bo = half4(1.0, 1.0, -1.0, -1.0);
    half4 Co = half4(1.5, 0.5, -0.5, 0.5);
    half4 Ax = half4(1.0, -1.0, -1.0, 1.0);
    half4 Bx = half4(0.5, 2.0, -0.5, -2.0);
    half4 Cx = half4(1.0, 1.0, -0.5, 0.0);
    half4 Ay = half4(1.0, -1.0, -1.0, 1.0);
    half4 By = half4(2.0, 0.5, -2.0, -0.5);
    half4 Cy = half4(2.0, 0.0, -1.0, 0.5);
        
    // These inequations define the line below which interpolation occurs.
    fx.x = (Ao.x * fp.y + Bo.x * fp.x > Co.x);
    fx.y = (Ao.y * fp.y + Bo.y * fp.x > Co.y);
    fx.z = (Ao.z * fp.y + Bo.z * fp.x > Co.z);
    fx.w = (Ao.w * fp.y + Bo.w * fp.x > Co.w);
        
    fx_left.x = (Ax.x * fp.y + Bx.x * fp.x > Cx.x);
    fx_left.y = (Ax.y * fp.y + Bx.y * fp.x > Cx.y);
    fx_left.z = (Ax.z * fp.y + Bx.z * fp.x > Cx.z);
    fx_left.w = (Ax.w * fp.y + Bx.w * fp.x > Cx.w);
        
    fx_up.x = (Ay.x * fp.y + By.x * fp.x > Cy.x);
    fx_up.y = (Ay.y * fp.y + By.y * fp.x > Cy.y);
    fx_up.z = (Ay.z * fp.y + By.z * fp.x > Cy.z);
    fx_up.w = (Ay.w * fp.y + By.w * fp.x > Cy.w);
        
    ir_lv1      = ((e != f) && (e != h));
    ir_lv2_left = ((e != g) && (d != g));
    ir_lv2_up   = ((e != c) && (b != c));
        
    half4 w1 = weighted_distance(e, c, g, i, h5, f4, h, f);
    half4 w2 = weighted_distance(h, d, i5, f, i4, b, e, i);
    half4 df_fg = df(f, g);
    half4 df_hc = df(h, c);
    half4 t1 = (coef * df_fg);
    half4 t2 = df_hc;
    half4 t3 = df_fg;
    half4 t4 = (coef * df_hc);
        
    edr      = (w1 < w2)  && ir_lv1;
    edr_left = (t1 <= t2) && ir_lv2_left;
    edr_up   = (t4 <= t3) && ir_lv2_up;
        
    nc = (edr && (fx || (edr_left && fx_left) || (edr_up && fx_up)));
        
    t1 = df(e, f);
    t2 = df(e, h);
    px = t1 <= t2;
        
    half3 res =
        nc.x ? px.x ? F : H :
        nc.y ? px.y ? B : F :
        nc.z ? px.z ? D : B :
        nc.w ? px.w ? H : D : E;
        
    outTexture.write(half4(res,1.0), gid);
}
#endif


#if 0
#define CORNER_A
#define XBR_SCALE 2.0
#define XBR_Y_WEIGHT 48.0
#define XBR_EQ_THRESHOLD 25.0
#define XBR_LV2_COEFFICIENT 2.0

half4 df(half4 A, half4 B)
{
    return half4(abs(A-B));
}

half c_df(half3 c1, half3 c2)
{
    half3 df = abs(c1 - c2);
    return df.r + df.g + df.b;
}

bool4 eq(half4 A, half4 B)
{
    return (df(A, B) < half4(XBR_EQ_THRESHOLD));
}

half4 weighted_distance(half4 a, half4 b, half4 c, half4 d, half4 e, half4 f, half4 g, half4 h)
{
    return (df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0*df(g,h));
}

kernel void xbrupscaler(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                        texture2d<half, access::write> outTexture  [[ texture(1) ]],
                        uint2                          gid         [[ thread_position_in_grid ]])
{
    const half3 Y = half3(0.2126, 0.7152, 0.0722);
    const half4 Ao = half4( 1.0, -1.0, -1.0, 1.0 );
    const half4 Bo = half4( 1.0,  1.0, -1.0,-1.0 );
    const half4 Co = half4( 1.5,  0.5, -0.5, 0.5 );
    const half4 Ax = half4( 1.0, -1.0, -1.0, 1.0 );
    const half4 Bx = half4( 0.5,  2.0, -0.5,-2.0 );
    const half4 Cx = half4( 1.0,  1.0, -0.5, 0.0 );
    const half4 Ay = half4( 1.0, -1.0, -1.0, 1.0 );
    const half4 By = half4( 2.0,  0.5, -2.0,-0.5 );
    const half4 Cy = half4( 2.0,  0.0, -1.0, 0.5 );
    const half4 Ci = half4(0.25, 0.25, 0.25, 0.25);
    
    bool4 edri, edr, edr_left, edr_up, px; // px = pixel, edr = edge detection rule
    bool4 interp_restriction_lv0, interp_restriction_lv1, interp_restriction_lv2_left, interp_restriction_lv2_up;
    half4 fx, fx_left, fx_up; // inequations of straight lines.
    
    half4 delta         = half4(1.0/XBR_SCALE, 1.0/XBR_SCALE, 1.0/XBR_SCALE, 1.0/XBR_SCALE);
    half4 deltaL        = half4(0.5/XBR_SCALE, 1.0/XBR_SCALE, 0.5/XBR_SCALE, 1.0/XBR_SCALE);
    half4 deltaU        = deltaL.yxwz;
    
    /*
    bool4 edr, edr_left, edr_up, px;        // px = pixel, edr = edge detection rule
    bool4 ir_lv1, ir_lv2_left, ir_lv2_up;
    bool4 nc;                               // new color
    bool4 fx, fx_left, fx_up;               // inequations of straight lines
    */
    
    half2 fp = fract(half2(gid) / SCALE_FACTOR);
    uint2 ggid = gid / SCALE_FACTOR;
    
    half3 A  = inTexture.read(ggid + uint2(-1,-1)).xyz;
    half3 B  = inTexture.read(ggid + uint2( 0,-1)).xyz;
    half3 C  = inTexture.read(ggid + uint2( 1,-1)).xyz;
    half3 D  = inTexture.read(ggid + uint2(-1, 0)).xyz;
    half3 E  = inTexture.read(ggid + uint2( 0, 0)).xyz;
    half3 F  = inTexture.read(ggid + uint2( 1, 0)).xyz;
    half3 G  = inTexture.read(ggid + uint2(-1, 1)).xyz;
    half3 H  = inTexture.read(ggid + uint2( 0, 1)).xyz;
    half3 I  = inTexture.read(ggid + uint2( 1, 1)).xyz;
    half3 A1 = inTexture.read(ggid + uint2(-1,-2)).xyz;
    half3 C1 = inTexture.read(ggid + uint2( 1,-2)).xyz;
    half3 A0 = inTexture.read(ggid + uint2(-2,-1)).xyz;
    half3 G0 = inTexture.read(ggid + uint2(-2, 1)).xyz;
    half3 C4 = inTexture.read(ggid + uint2( 2,-1)).xyz;
    half3 I4 = inTexture.read(ggid + uint2( 2, 1)).xyz;
    half3 G5 = inTexture.read(ggid + uint2(-1, 2)).xyz;
    half3 I5 = inTexture.read(ggid + uint2( 1, 2)).xyz;
    half3 B1 = inTexture.read(ggid + uint2( 0,-2)).xyz;
    half3 D0 = inTexture.read(ggid + uint2(-2, 0)).xyz;
    half3 H5 = inTexture.read(ggid + uint2( 0, 2)).xyz;
    half3 F4 = inTexture.read(ggid + uint2( 2, 0)).xyz;
    
  
    half4 b = XBR_Y_WEIGHT * Y * half4x3(B, D, H, F);
    half4 c = XBR_Y_WEIGHT * Y * half4x3(C, A, G, I);
    half4 e = XBR_Y_WEIGHT * Y * half4x3(E, E, E, E);
    half4 d = b.yzwx;
    half4 f = b.wxyz;
    half4 g = c.zwxy;
    half4 h = b.zwxy;
    half4 i = c.wxyz;
    
    half4 i4 = XBR_Y_WEIGHT * Y * half4x3(I4, C1, A0, G5);
    half4 i5 = XBR_Y_WEIGHT * Y * half4x3(I5, C4, A1, G0);
    half4 h5 = XBR_Y_WEIGHT * Y * half4x3(H5, F4, B1, D0);
    half4 f4 = h5.yzwx;
    
    // These inequations define the line below which interpolation occurs.
    fx      = (Ao*fp.y+Bo*fp.x);
    fx_left = (Ax*fp.y+Bx*fp.x);
    fx_up   = (Ay*fp.y+By*fp.x);
    
    interp_restriction_lv1 = interp_restriction_lv0 = ((e!=f) && (e!=h));
    
#ifdef CORNER_B
    interp_restriction_lv1      = (interp_restriction_lv0  &&  ( !eq(f,b) && !eq(h,d) || eq(e,i) && !eq(f,i4) && !eq(h,i5) || eq(e,g) || eq(e,c) ) );
#endif
#ifdef CORNER_D
    float4 c1 = i4.yzwx;
    float4 g0 = i5.wxyz;
    interp_restriction_lv1      = (interp_restriction_lv0  &&  ( !eq(f,b) && !eq(h,d) || eq(e,i) && !eq(f,i4) && !eq(h,i5) || eq(e,g) || eq(e,c) ) && (f!=f4 && f!=i || h!=h5 && h!=i || h!=g || f!=c || eq(b,c1) && eq(d,g0)));
#endif
#ifdef CORNER_C
    interp_restriction_lv1      = (interp_restriction_lv0  && ( !eq(f,b) && !eq(f,c) || !eq(h,d) && !eq(h,g) || eq(e,i) && (!eq(f,f4) && !eq(f,i4) || !eq(h,h5) && !eq(h,i5)) || eq(e,g) || eq(e,c)) );
#endif
    
    interp_restriction_lv2_left = ((e!=g) && (d!=g));
    interp_restriction_lv2_up   = ((e!=c) && (b!=c));
    
    half4 fx45i = saturate((fx      + delta  -Co - Ci)/(2*delta ));
    half4 fx45  = saturate((fx      + delta  -Co     )/(2*delta ));
    half4 fx30  = saturate((fx_left + deltaL -Cx     )/(2*deltaL));
    half4 fx60  = saturate((fx_up   + deltaU -Cy     )/(2*deltaU));
    
    half4 wd1 = weighted_distance( e, c, g, i, h5, f4, h, f);
    half4 wd2 = weighted_distance( h, d, i5, f, i4, b, e, i);
    
    edri     = (wd1 <= wd2) && interp_restriction_lv0;
    edr      = (wd1 <  wd2) && interp_restriction_lv1;
#ifdef CORNER_A
    edr      = edr && (!edri.yzwx || !edri.wxyz);
    edr_left = ((XBR_LV2_COEFFICIENT*df(f,g)) <= df(h,c)) && interp_restriction_lv2_left && edr && (!edri.yzwx && eq(e,c));
    edr_up   = (df(f,g) >= (XBR_LV2_COEFFICIENT*df(h,c))) && interp_restriction_lv2_up && edr && (!edri.wxyz && eq(e,g));
#endif
#ifndef CORNER_A
    edr_left = ((XBR_LV2_COEFFICIENT*df(f,g)) <= df(h,c)) && interp_restriction_lv2_left && edr;
    edr_up   = (df(f,g) >= (XBR_LV2_COEFFICIENT*df(h,c))) && interp_restriction_lv2_up && edr;
#endif
    
    fx45  = half4(edr)*fx45;
    fx30  = half4(edr_left)*fx30;
    fx60  = half4(edr_up)*fx60;
    fx45i = half4(edri)*fx45i;
    
    px = (df(e,f) <= df(e,h));
    
    half4 maximos = max(max(fx30, fx60), max(fx45, fx45i));
    
    half3 res1 = E;
    res1 = mix(res1, mix(H, F, px.x), maximos.x);
    res1 = mix(res1, mix(B, D, px.z), maximos.z);
    
    half3 res2 = E;
    res2 = mix(res2, mix(F, B, px.y), maximos.y);
    res2 = mix(res2, mix(D, H, px.w), maximos.w);
    
    half3 res = mix(res1, res2, step(c_df(E, res1), c_df(E, res2)));
    
    outTexture.write(half4(res,1.0), gid);
}
#endif


// xBR upscaler (based on xbr.js by carlos.ascari)

// constant half coef = 2.0;
constant half3 yuv_weighted = half3(14.352, 28.176, 5.472);

half4 df(half4 A, half4 B)
{
    return abs(A - B);
}

half d(half3 pixelA, half3 pixelB)
{
    const half Y_WEIGHT = 48;
    const half U_WEIGHT = 7;
    const half V_WEIGHT = 6;
    
    half r = abs(pixelA.r - pixelB.r);
    half b = abs(pixelA.b - pixelB.b);
    half g = abs(pixelA.g - pixelB.g);
    half y = r *  .299000 + g *  .587000 + b *  .114000;
    half u = r * -.168736 + g * -.331264 + b *  .500000;
    half v = r *  .500000 + g * -.418688 + b * -.081312;
    half weight = (y * Y_WEIGHT) + (u * U_WEIGHT ) + (v * V_WEIGHT);
    return weight;
}

kernel void xbrupscaler(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                        texture2d<half, access::write> outTexture  [[ texture(1) ]],
                        uint2                          gid         [[ thread_position_in_grid ]])
{
    
    if (gid.x % SCALE_FACTOR != 0 || gid.y % 2 != 0) return;
    
    // half2 fp = fract(half2(gid) / SCALE_FACTOR);
    uint2 ggid = gid / SCALE_FACTOR;
    
    half3 A  = inTexture.read(ggid + uint2(-1,-1)).xyz;
    half3 matrix4 = A;
    half3 B  = inTexture.read(ggid + uint2( 0,-1)).xyz;
    half3 matrix5 = B;
    half3 C  = inTexture.read(ggid + uint2( 1,-1)).xyz;
    half3 matrix6 = C;
    half3 D  = inTexture.read(ggid + uint2(-1, 0)).xyz;
    half3 matrix9 = D;
    half3 E  = inTexture.read(ggid + uint2( 0, 0)).xyz;
    half3 matrix10 = E;
    half3 F  = inTexture.read(ggid + uint2( 1, 0)).xyz;
    half3 matrix11 = F;
    half3 G  = inTexture.read(ggid + uint2(-1, 1)).xyz;
    half3 matrix14 = G;
    half3 H  = inTexture.read(ggid + uint2( 0, 1)).xyz;
    half3 matrix15 = H;
    half3 I  = inTexture.read(ggid + uint2( 1, 1)).xyz;
    half3 matrix16 = I;
    half3 A1 = inTexture.read(ggid + uint2(-1,-2)).xyz;
    half3 matrix0 = A1;
    half3 C1 = inTexture.read(ggid + uint2( 1,-2)).xyz;
    half3 matrix2 = C1;
    half3 A0 = inTexture.read(ggid + uint2(-2,-1)).xyz;
    half3 matrix3 = A0;
    half3 G0 = inTexture.read(ggid + uint2(-2, 1)).xyz;
    half3 matrix13 = G0;
    half3 C4 = inTexture.read(ggid + uint2( 2,-1)).xyz;
    half3 matrix7 = C4;
    half3 I4 = inTexture.read(ggid + uint2( 2, 1)).xyz;
    half3 matrix17 = I4;
    half3 G5 = inTexture.read(ggid + uint2(-1, 2)).xyz;
    half3 matrix18 = G5;
    half3 I5 = inTexture.read(ggid + uint2( 1, 2)).xyz;
    half3 matrix20 = I5;
    half3 B1 = inTexture.read(ggid + uint2( 0,-2)).xyz;
    half3 matrix1 = B1;
    half3 D0 = inTexture.read(ggid + uint2(-2, 0)).xyz;
    half3 matrix8 = D0;
    half3 H5 = inTexture.read(ggid + uint2( 0, 2)).xyz;
    half3 matrix19 = H5;
    half3 F4 = inTexture.read(ggid + uint2( 2, 0)).xyz;
    half3 matrix12 = F4;

    half d_10_9     = d(matrix10, matrix9);
    half d_10_5       = d(matrix10, matrix5);
    half d_10_11      = d(matrix10, matrix11);
    half d_10_15     = d(matrix10, matrix15);
    half d_10_14     = d(matrix10, matrix14);
    half d_10_6     = d(matrix10, matrix6);
    half d_4_8     = d(matrix4,  matrix8);
    half d_4_1     = d(matrix4,  matrix1);
    half d_9_5     = d(matrix9,  matrix5);
    half d_9_15     = d(matrix9,  matrix15);
    half d_9_3     = d(matrix9,  matrix3);
    half d_5_11     = d(matrix5,  matrix11);
    half d_5_0     = d(matrix5,  matrix0);
    half d_10_4     = d(matrix10, matrix4);
    half d_10_16     = d(matrix10, matrix16);
    half d_6_12     = d(matrix6,  matrix12);
    half d_6_1    = d(matrix6,  matrix1);
    half d_11_15     = d(matrix11, matrix15);
    half d_11_7     = d(matrix11, matrix7);
    half d_5_2     = d(matrix5,  matrix2);
    half d_14_8     = d(matrix14, matrix8);
    half d_14_19     = d(matrix14, matrix19);
    half d_15_18     = d(matrix15, matrix18);
    half d_9_13     = d(matrix9,  matrix13);
    half d_16_12     = d(matrix16, matrix12);
    half d_16_19     = d(matrix16, matrix19);
    half d_15_20     = d(matrix15, matrix20);
    half d_15_17     = d(matrix15, matrix17);
    
    // Top Left Edge Detection Rule
    half a1 = (d_10_14 + d_10_6 + d_4_8  + d_4_1 + (4 * d_9_5));
    half b1 = ( d_9_15 +  d_9_3 + d_5_11 + d_5_0 + (4 * d_10_4));
    if (a1 < b1)
    {
        half3 new_pixel = (d_10_9 <= d_10_5) ? matrix9 : matrix5;
        // var blended_pixel = blend(new_pixel, matrix[10], .5)
        outTexture.write(half4(new_pixel,1.0), gid);
        // sPixelView[((y * SCALE) * scaledWidth) + (x * SCALE)] = blended_pixel.value
    }
    else
    {
        outTexture.write(half4(matrix10,1.0), gid);
        // sPixelView[((y * SCALE) * scaledWidth) + (x * SCALE)] = matrix[10].value
    }
    
    // Top Right Edge Detection Rule
    half a2 = (d_10_16 + d_10_4 + d_6_12 + d_6_1 + (4 * d_5_11));
    half b2 = (d_11_15 + d_11_7 +  d_9_5 + d_5_2 + (4 * d_10_6));
    if (a2 < b2)
    {
        half3 new_pixel= (d_10_5 <= d_10_11) ? matrix5 : matrix11;
        outTexture.write(half4(new_pixel, 1.0), gid + uint2(1,0));
        // var blended_pixel = blend(new_pixel, matrix[10], .5)
        // sPixelView[((y * SCALE) * scaledWidth) + (x * SCALE + 1)] = blended_pixel.value
    }
    else
    {
        outTexture.write(half4(matrix10, 1.0), gid + uint2(1,0));
        // sPixelView[((y * SCALE) * scaledWidth) + (x * SCALE + 1)] = matrix[10].value
    }
    
    // Bottom Left Edge Detection Rule
    half a3 = (d_10_4 + d_10_16 +  d_14_8 + d_14_19 + (4 * d_9_15));
    half b3 = ( d_9_5 +  d_9_13 + d_11_15 + d_15_18 + (4 * d_10_14));
    if (a3 < b3)
    {
        half3 new_pixel = (d_10_9 <= d_10_15) ? matrix9 : matrix15;
        outTexture.write(half4(new_pixel, 1.0), gid + uint2(0,1));
        // var blended_pixel = blend(new_pixel, matrix[10], .5)
        // var index = ((y * SCALE + 1) * scaledWidth) + (x * SCALE)
        // sPixelView[index] = blended_pixel.value
    }
    else
    {
        outTexture.write(half4(matrix10, 1.0), gid + uint2(0,1));
        // var index = ((y * SCALE + 1) * scaledWidth) + (x * SCALE)
        // sPixelView[index] = matrix[10].value
    }
    
    // Bottom Right Edge Detection Rule
    half a4 = (d_10_6 + d_10_14 + d_16_12 + d_16_19 + (4 * d_11_15));
    half b4 = (d_9_15 + d_15_20 + d_15_17 +  d_5_11 + (4 * d_10_16));
    if (a4 < b4)
    {
        half3 new_pixel= (d_10_11 <= d_10_15) ? matrix11 : matrix15;
        outTexture.write(half4(new_pixel, 1.0), gid + uint2(1,1));
        // var blended_pixel = blend(new_pixel, matrix[10], .5)
        // sPixelView[((y * SCALE + 1) * scaledWidth) + (x * SCALE + 1)] = blended_pixel.value
    }
    else
    {
        outTexture.write(half4(matrix10, 1.0), gid + uint2(1,1));
        // sPixelView[((y * SCALE + 1) * scaledWidth) + (x * SCALE + 1)] = matrix[10].value
    }
}

//
// Scanline filters
//

/*
struct CrtParameters {
    float scanlineWeight;
    float scanlineBrightness;
};
*/

kernel void scanlines(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                      texture2d<half, access::write> outTexture  [[ texture(1) ]],
                      constant ShaderOptions         &options    [[ buffer(0) ]],
                      uint2                          gid         [[ thread_position_in_grid ]])
{
    half4 color = inTexture.read(uint2(gid.x, gid.y));
    if (((gid.y + 1) % 4) < 2) {
        color *= options.scanlineBrightness;
    }
    outTexture.write(color, gid);
}

//
// RGB splitter
//

/*
struct BloomUniforms {
    float bloomBrightness;
    float bloomWeight;
};
*/

kernel void split(texture2d<half, access::read>  t_in        [[ texture(0) ]],
                  texture2d<half, access::write> t_out_r     [[ texture(1) ]],
                  texture2d<half, access::write> t_out_g     [[ texture(2) ]],
                  texture2d<half, access::write> t_out_b     [[ texture(3) ]],
                  constant ShaderOptions         &options    [[ buffer(0) ]],
                  uint2                          gid         [[ thread_position_in_grid ]])
{

    half4 color = t_in.read(uint2(gid.x, gid.y));
    
    t_out_r.write(half4(color.r, 0, 0, 0), gid);
    t_out_g.write(half4(0, color.g, 0, 0), gid);
    t_out_b.write(half4(0, 0, color.b, 0), gid);
}


//
// Bypass filter
//

kernel void bypass(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                   texture2d<half, access::write> outTexture  [[ texture(1) ]],
                   uint2                          gid         [[ thread_position_in_grid ]])
{
    half4 result = inTexture.read(uint2(gid.x, gid.y));
    outTexture.write(result, gid);
}
