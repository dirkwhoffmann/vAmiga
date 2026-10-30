# ROM Panel

Use this panel to add ROM images to the emulator.

![Rom Panel](images/romPanel.png "Rom Panel")

When vAmiga is started for the first time, you will be taken directly to this panel. This is the place where you will manage an essential part of your virtual Amiga: The Kickstart ROM.

## Kickstart ROM

ROM images are added via drag and drop. To install a Kickstart ROM, simply drop a ROM image into the upper drop zone. Immediately after the image has been dropped, vAmiga performs a two-stage check. In the first stage, the file size and the values of some magic bytes are verified. If the file size does not match or the magic byte sequence is unknown to the emulator, the ROM image is rejected. Otherwise, the emulator continues by comparing the CRC checksum of the ROM image with the entries of a small database. When the CRC checksum is found, the name of the ROM is displayed along with some other information such as the release date. In the screenshot above you can see that vAmiga has recognized the dragged-in file as an original Commodore Kickstart ROM 1.3. If the CRC checksum is unknown to the emulator, vAmiga classifies the ROM as *Unknown*. You can still start the emulator with it, but there is no guarantee that the emulator will work as expected.

Please note that vAmiga does not ship with any of the original Commodore ROMs. All Amiga ROMs are the intellectual property Cloanto™ and distributed for a fee. However, if you have purchased a Kickstart ROM legally, you can use it in vAmiga with no issues.

If you don't have an original Kickstart at hand, you may choose to install the AROS ROM. The Amiga Research Operating System ROM is an open-source Kickstart variant which is freely redistributable. Two revisions of the AROS ROM are shipped with the emulator, which can be installed using the dropdown menu in the lower left corner. Unfortunately, not all Amiga programs are compatible with AROS, which is why I strongly recommend buying original ROMs.

## Extension ROM

In case you decide to resort to the AROS Kickstart replacement, you will notice that there will be two ROMs installed in your virtual Amiga. This is due to the size of the AROS ROM, which significantly exceeds the original Kickstart size of 512 KB. For this reason, AROS has been split into two separate ROMs. The first one appears in the same memory range as the original Kickstart, the second one appears in the address range $E00000 - $E7FFFF.

ROM extensions can also be installed manually by dropping a ROM image into the lower drop zone. Unlike the Kickstart ROM, which always resides in the same address range, the expansion ROM can be mapped to different memory areas. As mentioned above, the AROS expansion ROM must be visible to $E0000 to function. Other expansion ROMs, however, are expected to be visible in the $F00000 - $F7FFFF address range. You can set the address range manually by selecting the memory range from the popup menu next to the expansion ROM.
