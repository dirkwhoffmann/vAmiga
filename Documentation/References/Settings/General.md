# General Panel

Use this panel to adjust general settings, such as the screenshot and video format, as well as the behavior in warp or fullscreen mode.

![General Panel](images/generalPanel.png "General Panel")

## Snapshots

vAmiga can be configured to take a snapshot in regular intervals. In the firt place, this option has been added for mastering difficult games. Please keep in mind that snapshots are intended to be used as save-points. They are not meant as a persistent storage format, because the snapshot format is a moving target. A snapshot created with the current version will most likely not be readable in the next version.

## Screenshots

The options in this categery allow the user to adjust the source, the layout, and the target format of a screenshot. 

## Screen captures

vAmiga features a screen recorder which utilizes FFmpeg as backend. The options in this category allow the user to ajust various recording parameters. 

## Warp mode

In warp mode, vAmiga runs as fast as possible, i.e., it does no longer put the emulator thread to sleep. Warp mode can be configured as always off, always on, or automatically activated. In auto-mode, warp mode is switched on and off with the floppy drive motor logic. This mode works pretty well most of the time as it considerably reduced the waiting time during disk accesses. However, some games do not turn off the drive motor even if no data is being transferred. In these cases, auto-mode has to be switched off manually by the user. 

## Full screen mode

The first option let's you choose the full screen layout. The second options determines if the ESC key should be availabe as an emulation key or be used as a control key for exiting fullscreen mode. 
    
## Miscellaneous

The options in this category tell vAmiga how to react on certain events such as ejecting a disk or closing an emulator window. E.g., one option decides how vAmiga behaves when a floppy which contains unsaved data is being ejected. Please keep in mind that unlike other emulators, vAmiga does not modify any media file by itself. E.g., if an ADF has been inserted into a floppy drive, you can alter the disk inside vAmiga without hesitation. The original ADF will be untouched. As a drawback, however, you need to be extra cautious to export a floppy disk if you want to keep the modified contents. 