# Memory Panel

Use this panel to configure the memory configuration and basic access properties. 

![Memory Panel](images/memoryPanel.png "Memory Panel")

## Ram

The Amiga distinguishes three types of random access memories: 

- **Chip Ram**

  Chip RAM refers to the factory-installed memory that is located on the motherboard. It is the most flexible Ram, as it can be accessed by both the CPU and the custom chipset. The Amiga 1000 had a limited amount of 256 KB on board, which could be expanded by a 256 KB Ram module plugged into an expansion slot which was located on the front of the computer case. The Amiga 500, which was released later, was shipped with 512 KB straight from the factory. The Amiga 2000 was released in different versions over time. The original Amiga 2000 shipped with 512 KB, just like the Amiga 500 did. Later versions were equipped with 1 MB. 

- **Slow Ram** 

  The Amiga supports two types of Ram extensions. The first type is *Slow Ram*, which is also called *Ranger Ram*. In case of the Amiga 500, Slow Ram is added by inserting a memory expansion card into the trapdoor slot at the bottom of the computer case.  

   Slow Ram has a somewhat bad reputation because it combines the disadvantages from two worlds. Firstly, it cannot be accessed by the custom chips. As a result, it cannot be used for any data that is processed directly by the Blitter or Copper. Second, it uses the same memory bus as Chip Ram. This means that Slow Ram cannot be accessed during DMA cycles, even though the Ram itself is not accessible via DMA.

- **Fast Ram** 

   Fast Ram is the third type of Ram expansion. This type of Ram is directly connected to the CPU, which means that it is not accessible to the custom chips, just like Slow Ram isn't. However, since Fast Ram is not connected to the Chip Ram bus, the CPU can access this memory type in parallel to DMA activity. This is where the name Fast Ram stems from. The memory accesses themselves are not accelerated compared to Chip Ram or Slow Ram. 
   
   The default setting is 0 KB, because some Amiga programs refuse to work when Fast Ram is present.

Since 512 KB Chip Ram was very little and Fast Ram was very expensive, 512 KB of Chip Ram and 512 KB of Slow Ram was a common memory configuration at that time. Only a few users owned a Fast Ram memory expansion due to its high price.

## Memory layout

- **Bank map**

  The original Amigas use a 24-bit address bus, which means that they are capable of addressing 16 MB of memory. We can think of this memory as being divided into chunks of 64 KB, which we refer to as *memory banks*. The bank map determines where specific components are mapped in or mirrored to. For example, Chip Ram always starts at address $000000, while Slow Ram usually shows up at address $C00000. Mirroring means that we can access the same destination, such as the register for setting the background color, through several different addresses. Thus a programmer could just as easily use an address from the mirrored range instead of the officially documented address, and many programmers have done so in the past. Unfortunately, the mirrored ranges are not the same in all Amiga models, which means that we have to tell the emulator which memory layout should be used. This is the purpose of this configuration option. 
  
  vAmiga supports four different bank maps: **A500**, **A1000**, **A2000A**, and **A2000B**. The bank maps mainly differ in the memory location of the RTC register space. In the first Amigas, namely the A1000 and the A2000A, the real-time clock was accessed through memory bank $D8. In the A500 and the most common A2000 model, the A2000B, Commodore mapped the real-time clock into memory bank $DC. 

  Other differences are related to mirroring. For example, the official location of the custom register space is memory bank $DF. However, this bank is usually mirrored multiple times, which means that the custom registers can also be accessed via other memory addresses. E.g., on older Amigas, the custom register space is mirrored in bank $DC, which is the same bank where later Amigas map in the real-time clock.

- **Init Pattern**

  This configuration option determines how the memory should be initialized at startup. You can choose between random values, all zeros or all ones.
 
- **Unmapped area**

  This option tells vAmiga what to put on the data bus when an unmapped memory location is accessed. Besides telling the emulator to leave the data bus in a floating state, you can choose to return all zeros or all ones. The first option is close to what happens in a real Amiga, but the current implementation is not 100% accurate. Any advice on how to improve emulation accuracy is very welcome. 

## Chipset features

- **Emulate Slow Ram Mirror** 

  The ECS variants of Agnus have an interesting capability the OCS variants don't have. When an ECS Agnus is connected to 512 KB of Chip RAM and 512 KB of Slow RAM, it mirrors the Slow RAM pages into the second Chip Ram segment, making Slow Ram accessible for DMA. Now, the Blitter and Copper have full access to both Rams. *Move Any Mountain* by Lazy Bones is an Amiga demo that relies on this feature. It was presented at the POLARIS computer party in 1993 and does not work when an OCS Agnus or an incompatible memory configuration is used.

  This configuration option allows the user to enable or disable this feature. If an OCS Agnus is emulated or if the selected memory configuration does not match the 512 KB Chip Ram + 512 Slow Ram configuration, the selection has no effect. No mirroring will take place.

- **Emulate Slow Ram Bus delays**

  As mentioned before, the CPU needs a free DMA cycle to access Slow Ram. By default, vAmiga emulates this behavior. Disabling this option removes this restriction, allowing the CPU to access Slow Ram at the same pace as Fast Ram. Please note that accelerating Slow Ram accesses may cause incompatibilities in rare occasions, as the emulated behaviour now deviates from what we see on the real machine.
