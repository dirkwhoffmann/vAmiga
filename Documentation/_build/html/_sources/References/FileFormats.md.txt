# Supported File Formats

## Floppy disks

vAmiga supports the following file formats for floppy disks:

- **ADF format**

  ADF files contain a byte dump of all sectors of an Amiga diskette. The format is extremely simple in structure, since it does not store any header information. The ADF format is the most widely used format for Amiga floppy disks and almost all non-copy-protected titles are available in form of ADF files. 

- **Extended ADF format** 

  The extended ADF format has been created to store copy-protected disks or disks that were formatted in a non-standard format. It has a much more complicated structure than the standard ADF format and offers different methods to represent track data. For example, the format is able to store the uninterpreted MFM data stream of single tracks or the whole disk. 

- **DMS Format**

  DMS files store a digital image of an Amiga floppy disk, just like ADFs. However, the internal representation is much more complicated. The format is very popular in the demo scene. Thus, you will find many demos encoded in this format.

- **EXE Format**
	
  vAmiga also supports pure Amiga executables. When such a file is passed to the emulator, vAmiga creates a floppy disk on-the-fly and copies the executable to it. It also makes the floppy bootable by adding a boot block and a startup sequence that loads the executable.

- **IMG Format**

  Image files are floppy disks in MS-DOS format. Inserting such a floppy disk makes sense only if you use special software that can read such floppy disks.

- **Folders**

  vAmiga allows directories of the host computer to be used in the form of floppy disks. When a directory is provided by drag-and-drop, the emulator tries to write all files of the directory to an empty floppy disk. If the floppy disk has sufficient space for all files, the disk can be used inside the emulator or exported, e.g., in form of an ADF file.

## Hard drives

- **HDF Format** 

  HDF files are the equivalent of ADF files for hard disks. An HDF file contains a byte dump of all sectors of a hard disk, just as an ADF file contains a byte dump of all sectors of a floppy disk.
  
