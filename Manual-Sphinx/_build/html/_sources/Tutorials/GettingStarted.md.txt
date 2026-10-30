# Getting Started 

This tutorial describes how to launch vAmiga and install a Kickstart ROM.

## Installing vAmiga

vAmiga is a native Mac application and as easy to install as most other Mac software. Download the latest version from the main page and copy the executable into the application folder.

vAmiga's deployment target is **macOS 12 (Monterey)** which means that you can't run the app on earlier macOS releases. Please note that vAmiga is only extensively tested with the latest macOS version. Thus, compatibility problems may well occur with older macOS versions. 

Please keep in mind that vAmiga has been developed with limited work-power leaving no room for supporting older macOS releases. You can always extract older version from the source-code repository, but you will be on your own by doing so. 

## Installing a Kickstart ROM

Emulating an Amiga requires a Kickstart ROM. Since the original Amiga ROMs are the intellectual property of Cloanto™, they cannot be shipped with the emulator. Thus, when opening vAmiga for the first time, the emulator will greet you with a ROM dialog:

![Inspectors](images/roms1.png "Rom Dialog")

In this dialog, a Kickstart ROM can be added via drag and drop. If you are the legal owner of a ROM image, you can easily install it by dragging it over the ROM icon:

![Inspectors](images/roms2.png "Rom Dialog")

Once the ROM is installed, the emulator is ready to be powered on. To do so, click the power button and have fun with your new old Amiga:

![Inspectors](images/roms3.png "Rom Dialog")

In case you don't own a legal ROM, you can alternatively start vAmiga with the free AROS Kickstart replacement. AROS ROMs ship with the emulator and can be selected from the *Install ROM* popup-menu located in the lower left corner.

As you can see below, the popup menu is presently grayed out therefore inaccessible. This is to protect against accidental changes to the Kickstart ROM while the emulator is running. To access the menu, turn off the emulator first by clicking on the lock icon.

![Inspectors](images/roms4.png "Rom Dialog")

After the emulator has been powered off, the AROS ROMs can be installed with a single click:

![Inspectors](images/roms5.png "Rom Dialog")

Again, clicking the power button starts the emulator. With the free Kickstart ROM replacement installed, the AROS boot screen shows up:

![Inspectors](images/roms6.png "Rom Dialog")

Even though the AROS ROMs are a great achievement of the open source community, we recommend installing original Commodore ROMs as they offer much higher compatibility.

Please keep in mind that vAmiga will not remember the ROM settings by default, i.e., the next time the emulator is started, the ROM dialog will appear again. If you wish to use the currently installed ROM permanently, click the *Use as Default* button next to the AROS button.

## Inserting a floppy disk

Next, we'll show how to insert a floppy disk. Thanks to the diligent work of many Amiga enthusiasts, almost all Amiga software from back in the day has been saved from decay and translated into modern data formats. The most important data format for the Amiga is the ADF format, which is a digital image of an Amiga floppy disk. On the Internet you will find countless ADFs and most likely your beloved titles from back then, too. Please note that despite their age, most Amiga titles are protected by copyright and may not be used without the permission of the rights holder.

The most convenient way to insert a floppy disk is by drag and drop. When an ADF file is dragged into the emulator window, four drop zones appear representing drives DF0 through DF3. Simply drop the file onto the drive in which you want to insert the disk.

![Inserting a floppy disk](images/dropzone.png "Inserting a floppy disk")

Shortly after dropping the disk onto the drop zone for drive DF0, the Amiga recognizes the inserted disk and starts to boot. In the example above, we've inserted a bootable version of *Rink a Dink*, a popular Amiga demo which was published in 1993 by *Lemon*. The demo works flawlessly with the AROS ROMs as it takes over the machine entirely.

![Rink a dink](images/rinkadink.png "Rink a dink")
