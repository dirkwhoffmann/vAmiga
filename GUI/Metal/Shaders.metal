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
    float bloomRadius;
    float bloomBrightness;
    float bloomWeight;

    uint flicker;
    float flickerWeight;

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
// Merge shader data types
//

struct MergeUniforms {

    float longFrameScale;
    float shortFrameScale;
};

//
// Fragment shader data types
//

struct FragmentUniforms {
    
    float alpha;
    float white;
    uint dotMaskWidth;
    uint dotMaskHeight;
    uint scanlineDistance;
};

//
// Vertex shader
//

vertex ProjectedVertex vertex_main(const device InVertex *vertices [[buffer(0)]],
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

float3 scanlineWeight(uint2 pixel, uint height, float weight, float brightness, float bloom) {
    
    // Calculate distance to nearest scanline
    float dy = ((float(pixel.y % height) / float(height - 1)) - 0.5);
 
    // Calculate scanline weight
    float scanlineWeight = max(1.0 - dy * dy * 24 * weight, brightness);
    
    // Apply bloom effect an return
    float3 result = scanlineWeight * bloom;
    return result;
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
        color = float4(r.r, g.g, b.b, 1.0);
    } else {
        color = texture.sample(texSampler, float2(vert.texCoords.x, vert.texCoords.y));
    }
    
    // Apply bloom effect (if enabled)
    if (options.bloom) {
        float4 bloom_r = bloomTextureR.sample(texSampler, tc);
        float4 bloom_g = bloomTextureG.sample(texSampler, tc);
        float4 bloom_b = bloomTextureB.sample(texSampler, tc);
        float4 bColor = bloom_r + bloom_g + bloom_b;
        bColor = pow(bColor, options.bloomWeight) * options.bloomBrightness;
        color = saturate(color + bColor);
    }
    
    // Apply scanline effect (if emulation type matches)
    if (options.scanlines == 2) {
        color.rgb *= scanlineWeight(pixel,
                                    uniforms.scanlineDistance,
                                    options.scanlineWeight,
                                    options.scanlineBrightness,
                                    1.0);
    }

    // Apply dot mask effect
    if (options.dotMask) {
        uint xoffset = x % uniforms.dotMaskWidth;
        uint yoffset = y % uniforms.dotMaskHeight;
        float4 dotColor = dotMask.read(uint2(xoffset, yoffset));
        float4 gain = min(color, 1 - color) * dotColor;
        float4 loose = min(color, 1 - color) * 0.5 * (1 - dotColor);
        color += gain - loose;
    }
    
    color = mix(color, float4(1.0, 1.0, 1.0, 1.0), uniforms.white);
    return half4(color.r, color.g, color.b, color.a * uniforms.alpha);
}


//
// Texture merger
//

kernel void bypassmerger(texture2d<half, access::read>  inTexture   [[ texture(0) ]],
                         texture2d<half, access::write> outTexture  [[ texture(1) ]],
                         uint2                          gid         [[ thread_position_in_grid ]])
{
    half4 result = inTexture.read(uint2(gid.x, gid.y / 2));
    outTexture.write(result, gid);
}
    
kernel void merge(texture2d<half, access::read>  longFrame   [[ texture(0) ]],
                  texture2d<half, access::read>  shortFrame  [[ texture(1) ]],
                  texture2d<half, access::write> outTexture  [[ texture(2) ]],
                  constant MergeUniforms         &uniforms   [[ buffer(0) ]],
                  uint2                          gid         [[ thread_position_in_grid ]])
{
    half4 result;
    float s;
    
    if (gid.y % 2 == 0) {
        s = uniforms.longFrameScale;
        result = longFrame.read(uint2(gid.x, gid.y / 2));
    } else {
        s = uniforms.shortFrameScale;
        result = shortFrame.read(uint2(gid.x, gid.y / 2));
    }
    
    outTexture.write(result * vec<half,4>(s,s,s,1), gid);
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

//
// EPX upscaler (Eric's Pixel Expansion)
//

void doEPX(texture2d<half, access::write> out, uint2 gid,
           half4 A, half4 C, half4 P, half4 B, half4 D)
{
    //   A    --\ 1 2
    // C P B  --/ 3 4
    //   D
    // 1=P; 2=P; 3=P; 4=P;
    // IF C==A AND C!=D AND A!=B => 1=A
    // IF A==B AND A!=C AND B!=D => 2=B
    // IF D==C AND D!=B AND C!=A => 3=C
    // IF B==D AND B!=A AND D!=C => 4=D
    
    half4 r1 = (all(C == A) && any(C != D) && any(A != B)) ? A : P;
    half4 r2 = (all(A == B) && any(A != C) && any(B != D)) ? B : P;
    half4 r3 = (all(A == B) && any(A != C) && any(B != D)) ? C : P;
    half4 r4 = (all(B == D) && any(B != A) && any(D != C)) ? D : P;
    
    // DEBUGGING
    /*
    r1 = half4(1.0,0.0,0.0,1.0);
    r2 = half4(1.0,0.0,0.0,1.0);
    r3 = half4(1.0,0.0,0.0,1.0);
    r4 = half4(1.0,0.0,0.0,1.0);
    */
    
    out.write(r1, gid + uint2(0,0));
    out.write(r2, gid + uint2(1,0));
    out.write(r3, gid + uint2(0,1));
    out.write(r4, gid + uint2(1,1));
}

#if 0
kernel void inPlaceEpx(texture2d<half, access::read>  in   [[ texture(0) ]],
                       texture2d<half, access::write> out  [[ texture(1) ]],
                       uint2                          gid  [[ thread_position_in_grid ]])
{
    if((gid.x % SCALE_FACTOR != 0) || (gid.y % SCALE_FACTOR != 0))
        return;
    
    // Check for lores fragment (a == b == c == d)
    // a b
    // c d
    
    half4 a = in.read(gid + uint2(0,0));
    half4 b = in.read(gid + uint2(1,0));
    half4 c = in.read(gid + uint2(0,1));
    half4 d = in.read(gid + uint2(1,1));

    if (all(a == b && b == c && c == d)) {
        
        //   A
        // C P B
        //   D
        
        half4 A = in.read(gid + uint2( 0,-2));
        half4 C = in.read(gid + uint2(-2, 0));
        half4 P = in.read(gid + uint2( 0, 0));
        half4 B = in.read(gid + uint2( 2, 0));
        half4 D = in.read(gid + uint2( 0, 2));
        
        doEPX(out, gid, A, C, P, B, D);
        
    } else {
        
        // DEBUGGING
        /*
        a = half4(1.0,0.0,0.0,1.0);
        b = half4(1.0,0.0,0.0,1.0);
        c = half4(1.0,0.0,0.0,1.0);
        d = half4(1.0,0.0,0.0,1.0);
        */
        
        out.write(a, gid + uint2(0,0));
        out.write(b, gid + uint2(1,0));
        out.write(c, gid + uint2(0,1));
        out.write(d, gid + uint2(1,1));
    }
}
#endif

kernel void inPlaceEpx(texture2d<half, access::read>  in   [[ texture(0) ]],
                       texture2d<half, access::write> out  [[ texture(1) ]],
                       uint2                          gid  [[ thread_position_in_grid ]])
{
    // We only apply in-texture upscaling for lores lines. The emulator encodes
    // this information in a certain texture pixel.
    if (in.read(uint2(0, gid.y)).a == 1.0) {
        
        // This line has been marked as a hires line by the emulator
        out.write(in.read(gid), gid);
        // out.write(half4(1.0,0.0,0.0,1.0), gid);
        return;
    }

    if (gid.x % SCALE_FACTOR != 0 || gid.y % 2 != 0) return;
    
    // Check for lores fragment (a == b == c == d)
    // a b
    // c d
    
    half4 a = in.read(gid + uint2(0,0));
    half4 b = in.read(gid + uint2(1,0));
    half4 c = in.read(gid + uint2(0,1));
    half4 d = in.read(gid + uint2(1,1));

    if (all(a == b && b == c && c == d)) {
        
        //   A
        // C P B
        //   D
        
        half4 A = in.read(gid + uint2( 0,-2));
        half4 C = in.read(gid + uint2(-2, 0));
        half4 P = in.read(gid + uint2( 0, 0));
        half4 B = in.read(gid + uint2( 2, 0));
        half4 D = in.read(gid + uint2( 0, 2));
        
        doEPX(out, gid, A, C, P, B, D);
        
    } else {
        
        // DEBUGGING
        /*
        a = half4(1.0,0.0,0.0,1.0);
        b = half4(1.0,0.0,0.0,1.0);
        c = half4(1.0,0.0,0.0,1.0);
        d = half4(1.0,0.0,0.0,1.0);
        */
        
        out.write(a, gid + uint2(0,0));
        out.write(b, gid + uint2(1,0));
        out.write(c, gid + uint2(0,1));
        out.write(d, gid + uint2(1,1));
    }
}

kernel void epxupscaler(texture2d<half, access::read>  in   [[ texture(0) ]],
                        texture2d<half, access::write> out  [[ texture(1) ]],
                        uint2                          gid  [[ thread_position_in_grid ]])
{
    if((gid.x % SCALE_FACTOR != 0) || (gid.y % SCALE_FACTOR != 0))
        return;
    
    uint2 ggid = gid / 2;
    
    half4 A = in.read(ggid + uint2( 0,-1));
    half4 C = in.read(ggid + uint2(-1, 0));
    half4 P = in.read(ggid + uint2( 0, 0));
    half4 B = in.read(ggid + uint2( 1, 0));
    half4 D = in.read(ggid + uint2( 0, 1));
    
    doEPX(out, gid, A, C, P, B, D);
}

//
// xBR upscaler (based on xbr.js by carlos.ascari)
//

constant half3 yuv_weighted = half3(14.352, 28.176, 5.472);

half4 df(half4 A, half4 B)
{
    return abs(A - B);
}

half d(half3 pixelA, half3 pixelB)
{
    half3 rgb = abs(pixelA - pixelB);
    half weight = dot(yuv_weighted, rgb);
    return weight;
    
    /*
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
    */
}

void doXBR(texture2d<half, access::write> out, uint2 gid,
           half3 m0, half3 m1, half3 m2, half3 m3, half3 m4, half3 m5,
           half3 m6, half3 m7, half3 m8, half3 m9, half3 m10, half3 m11,
           half3 m12, half3 m13, half3 m14, half3 m15, half3 m16, half3 m17,
           half3 m18, half3 m19, half3 m20)
{
    half d_10_9    = d(m10, m9);
    half d_10_5    = d(m10, m5);
    half d_10_11   = d(m10, m11);
    half d_10_15   = d(m10, m15);
    half d_10_14   = d(m10, m14);
    half d_10_6    = d(m10, m6);
    half d_4_8     = d(m4,  m8);
    half d_4_1     = d(m4,  m1);
    half d_9_5     = d(m9,  m5);
    half d_9_15    = d(m9,  m15);
    half d_9_3     = d(m9,  m3);
    half d_5_11    = d(m5,  m11);
    half d_5_0     = d(m5,  m0);
    half d_10_4    = d(m10, m4);
    half d_10_16   = d(m10, m16);
    half d_6_12    = d(m6,  m12);
    half d_6_1     = d(m6,  m1);
    half d_11_15   = d(m11, m15);
    half d_11_7    = d(m11, m7);
    half d_5_2     = d(m5,  m2);
    half d_14_8    = d(m14, m8);
    half d_14_19   = d(m14, m19);
    half d_15_18   = d(m15, m18);
    half d_9_13    = d(m9,  m13);
    half d_16_12   = d(m16, m12);
    half d_16_19   = d(m16, m19);
    half d_15_20   = d(m15, m20);
    half d_15_17   = d(m15, m17);
    
    half3 pixel;
    const half blend = 0.5;
    
    // X-
    // --
    
    half a1 = d_10_14 + d_10_6 + d_4_8  + d_4_1 + (4 * d_9_5);
    half b1 =  d_9_15 +  d_9_3 + d_5_11 + d_5_0 + (4 * d_10_4);
    
    if (a1 < b1) {
        pixel = (d_10_9 <= d_10_5) ? m9 : m5;
        pixel = mix(pixel, m10, blend);
    } else {
        pixel = m10;
    }
    out.write(half4(pixel,1.0), gid);
    
    // -X
    // --
    
    half a2 = d_10_16 + d_10_4 + d_6_12 + d_6_1 + (4 * d_5_11);
    half b2 = d_11_15 + d_11_7 +  d_9_5 + d_5_2 + (4 * d_10_6);
    
    if (a2 < b2) {
        pixel = (d_10_5 <= d_10_11) ? m5 : m11;
        pixel = mix(pixel, m10, blend);
    } else {
        pixel = m10;
    }
    out.write(half4(pixel, 1.0), gid + uint2(1,0));
    
    // --
    // X-
    
    half a3 = d_10_4 + d_10_16 +  d_14_8 + d_14_19 + (4 * d_9_15);
    half b3 =  d_9_5 +  d_9_13 + d_11_15 + d_15_18 + (4 * d_10_14);
    
    if (a3 < b3) {
        pixel = (d_10_9 <= d_10_15) ? m9 : m15;
        pixel = mix(pixel, m10, blend);
    } else {
        pixel = m10;
    }
    out.write(half4(pixel, 1.0), gid + uint2(0,1));
    
    // --
    // -X
    
    half a4 = (d_10_6 + d_10_14 + d_16_12 + d_16_19 + (4 * d_11_15));
    half b4 = (d_9_15 + d_15_20 + d_15_17 +  d_5_11 + (4 * d_10_16));
    if (a4 < b4) {
        pixel = (d_10_11 <= d_10_15) ? m11 : m15;
        pixel = mix(pixel, m10, blend);
    } else {
        pixel = m10;
    }
    out.write(half4(pixel, 1.0), gid + uint2(1,1));
}
    
kernel void xbrupscaler(texture2d<half, access::read>  in   [[ texture(0) ]],
                        texture2d<half, access::write> out  [[ texture(1) ]],
                        uint2                          gid  [[ thread_position_in_grid ]])
{
    
    if (gid.x % SCALE_FACTOR != 0 || gid.y % SCALE_FACTOR != 0) return;
  
    //         -2   -1   +0   +1   +2
    //
    //            ----------------
    //  -2        |  0 |  1 |  2 |
    //       --------------------------
    //  -1   |  3 |  4 |  5 |  6 |  7 |
    //       --------------------------
    //   0   |  8 |  9 | 10 | 11 | 12 |
    //       --------------------------
    //  +1   | 13 | 14 | 15 | 16 | 17 |
    //       --------------------------
    //  +2        | 18 | 19 | 20 |
    //            ----------------
    
    uint2 ggid = gid / SCALE_FACTOR;
    
    half3 m0  = in.read(ggid + uint2(-1,-2)).xyz;
    half3 m1  = in.read(ggid + uint2( 0,-2)).xyz;
    half3 m2  = in.read(ggid + uint2( 1,-2)).xyz;
    half3 m3  = in.read(ggid + uint2(-2,-1)).xyz;
    half3 m4  = in.read(ggid + uint2(-1,-1)).xyz;
    half3 m5  = in.read(ggid + uint2( 0,-1)).xyz;
    half3 m6  = in.read(ggid + uint2( 1,-1)).xyz;
    half3 m7  = in.read(ggid + uint2( 2,-1)).xyz;
    half3 m8  = in.read(ggid + uint2(-2, 0)).xyz;
    half3 m9  = in.read(ggid + uint2(-1, 0)).xyz;
    half3 m10 = in.read(ggid + uint2( 0, 0)).xyz;
    half3 m11 = in.read(ggid + uint2( 1, 0)).xyz;
    half3 m12 = in.read(ggid + uint2( 2, 0)).xyz;
    half3 m13 = in.read(ggid + uint2(-2, 1)).xyz;
    half3 m14 = in.read(ggid + uint2(-1, 1)).xyz;
    half3 m15 = in.read(ggid + uint2( 0, 1)).xyz;
    half3 m16 = in.read(ggid + uint2( 1, 1)).xyz;
    half3 m17 = in.read(ggid + uint2( 2, 1)).xyz;
    half3 m18 = in.read(ggid + uint2(-1, 2)).xyz;
    half3 m19 = in.read(ggid + uint2( 0, 2)).xyz;
    half3 m20 = in.read(ggid + uint2( 1, 2)).xyz;
    
    doXBR(out, gid,
          m0,  m1,  m2,  m3,  m4,  m5,  m6,  m7,  m8,  m9,
          m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20);
}

kernel void inPlaceXbr(texture2d<half, access::read>  in   [[ texture(0) ]],
                       texture2d<half, access::write> out  [[ texture(1) ]],
                       uint2                          gid  [[ thread_position_in_grid ]])
{
    // We only apply in-texture upscaling for lores lines. The emulator encodes
    // this information in a certain texture pixel.
    if (in.read(uint2(0, gid.y)).g == 0) {
        
        // This line has been marked as a hires line by the emulator
        out.write(in.read(gid), gid);
        return;
    }
        
    if (gid.x % SCALE_FACTOR != 0 || gid.y % 2 != 0) return;

    //         -2   -1   +0   +1   +2
    //
    //            ----------------
    //  -2        |  0 |  1 |  2 |
    //       --------------------------
    //  -1   |  3 |  4 |  5 |  6 |  7 |
    //       --------------------------
    //   0   |  8 |  9 | 10 | 11 | 12 |
    //       --------------------------
    //  +1   | 13 | 14 | 15 | 16 | 17 |
    //       --------------------------
    //  +2        | 18 | 19 | 20 |
    //            ----------------
    
    half3 m0  = in.read(gid + 2 * uint2(-1,-2)).xyz;
    half3 m1  = in.read(gid + 2 * uint2( 0,-2)).xyz;
    half3 m2  = in.read(gid + 2 * uint2( 1,-2)).xyz;
    half3 m3  = in.read(gid + 2 * uint2(-2,-1)).xyz;
    half3 m4  = in.read(gid + 2 * uint2(-1,-1)).xyz;
    half3 m5  = in.read(gid + 2 * uint2( 0,-1)).xyz;
    half3 m6  = in.read(gid + 2 * uint2( 1,-1)).xyz;
    half3 m7  = in.read(gid + 2 * uint2( 2,-1)).xyz;
    half3 m8  = in.read(gid + 2 * uint2(-2, 0)).xyz;
    half3 m9  = in.read(gid + 2 * uint2(-1, 0)).xyz;
    half3 m10 = in.read(gid + 2 * uint2( 0, 0)).xyz;
    half3 m11 = in.read(gid + 2 * uint2( 1, 0)).xyz;
    half3 m12 = in.read(gid + 2 * uint2( 2, 0)).xyz;
    half3 m13 = in.read(gid + 2 * uint2(-2, 1)).xyz;
    half3 m14 = in.read(gid + 2 * uint2(-1, 1)).xyz;
    half3 m15 = in.read(gid + 2 * uint2( 0, 1)).xyz;
    half3 m16 = in.read(gid + 2 * uint2( 1, 1)).xyz;
    half3 m17 = in.read(gid + 2 * uint2( 2, 1)).xyz;
    half3 m18 = in.read(gid + 2 * uint2(-1, 2)).xyz;
    half3 m19 = in.read(gid + 2 * uint2( 0, 2)).xyz;
    half3 m20 = in.read(gid + 2 * uint2( 1, 2)).xyz;
    
    doXBR(out, gid,
          m0,  m1,  m2,  m3,  m4,  m5,  m6,  m7,  m8,  m9,
          m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20);
        
    // DEBUGGING
    /*
    a = half4(1.0,0.0,0.0,1.0);
    b = half4(1.0,0.0,0.0,1.0);
    c = half4(1.0,0.0,0.0,1.0);
    d = half4(1.0,0.0,0.0,1.0);
    out.write(a, gid + uint2(0,0));
    out.write(b, gid + uint2(1,0));
    out.write(c, gid + uint2(0,1));
    out.write(d, gid + uint2(1,1));
    */
}

//
// Scanline filters
//

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
