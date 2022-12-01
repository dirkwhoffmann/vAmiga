# Peripherals Panel

![Peripherals Panel](images/peripheralsPanel.png "Peripherals Panel")

## Floppy Drives

Just like a real Amiga, vAmiga supports up to four floppy drives. The internal drive is labeled df0. and always present. External drives are labeled df1. through df3. The internal drive is a 3.5" double-density drive which offers a storage capacity of 880 KB per disk. The external drives can be configured as double-density drives, just like the internal drive, or as high-density drives which offer twice the storage capacity. Back in the day, it was common to attach 5.25" drives to the Amiga because of the much lower price of 5.25" floppy disks. Such a drive offered half the capacity of a standard Amiga disk, namely 440 KB. These drives are not supported by vAmiga, yet. 

## Hard Drives

Thanks to GitHub user mras0, vAmiga is able to emulate hard drives, too. Up to four hard drive controllers are supported. Each virtual hard drive controller occupies as Zorro II slot and identifies itself to the Amiga via the AUTOCONFIG mechanism.

## Game Ports

The Amiga features two game ports for connecting input devices such as mice or joysticks. The connected input device can be selected from a drop-down list. By default, vAmiga offers you to leave the port empty, connect the internal mouse, or to connect a keyboard-emulated joystick. If an external mouse or compatible GamePad is attached, the device will appear in the drop-down list as another option.

## Serial Port

vAmiga offers the following options for the serial port: 

  - **No Device**

    No device is attached to the port. This is the default option. 


  - **Null Modem Cable**

    vAmiga offers to emulate a null modem cable. By opening two instances of vAmiga on your computer, you can connect them via the null modem cable and enjoy, e.g., a nice game of Battle Chess with two Amigas fighting against each other. 

  - **Loopback Cable** 

    A loopback cable connects the output pins of the serial with the input pins of the same port. Such a cable is required, e.g., to run the serial port test built into 'Amiga Test Kit': 

    ![Amiga Test Kit](images/AmigaTestKitSerial.png "Amiga Test Kit")
