// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DRIVE_INC
#define _AMIGA_DRIVE_INC

#include "AmigaComponent.h"
#include "Disk.h"

class Drive : public AmigaComponent {
    
    friend class DiskController;
        
    // This object emulates drive df<nr>
    const int nr;

    // Bookkeeping
    DriveConfig config;
    DriveInfo info;

    // Position of the currently transmitted identification bit
    u8 idCount;

    // Value of the currently transmitted identification bit
    bool idBit;

    /* Indicates if the motor is running at full speed
     * On a real drive, it can take up to one half second (500ms) until the
     * drive runs at full speed. We don't emulate  accurate timing here and
     * set the variable to true once the drive motor is switched on.
     *
     * TODO: MAKE it A COMPUTED VALUE:
     * bool motor() { motorOffCycle >= motorOnCycle; }
     */
    bool motor;

    // Records when the drive motor was switch on the last time
    Cycle motorOnCycle;

    // Records when the drive motor was switch off the last time
    Cycle motorOffCycle;

    /* Disk change status
     * This variable controls the /CHNG bit in the CIA A PRA register. Note
     * that the variable only changes its value under certain circumstances.
     * If a head movement pulse is sent and no disk is inserted, the variable
     * is set to false (which is also the reset value). It becomes true when
     * a disk is ejected.
     */
    bool dskchange;
    
    // A copy of the DSKLEN register
    u8 dsklen;
    
    // A copy of the PRB register of CIA B
    u8 prb;
    
    // The current drive head location
    DriveHead head;
    
    /* History buffer storing the most recently visited tracks.
     * The buffer is used to detect the polling head movements that are issued
     * by track disc device to detect a newly inserted disk.
     */
    u64 cylinderHistory;

public:
    
    // The currently inserted disk (NULL if the drive is empty)
    Disk *disk = NULL;

    
    //
    // Methods
    //

public:

    Drive(unsigned nr, Amiga& ref);

    DriveConfig getConfig() { return config; }
    DriveInfo getInfo();

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.type
        & config.speed;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & idCount
        & idBit
        & motor
        & motorOnCycle
        & motorOffCycle
        & dskchange
        & dsklen
        & prb
        & head.side
        & head.cylinder
        & head.offset
        & cylinderHistory;
    }

private:

    void _reset() override;
    void _ping() override;
    void _inspect() override;
    void _dumpConfig() override;
    void _dump() override;
    size_t _size() override;
    size_t _load(u8 *buffer) override;
    size_t _save(u8 *buffer) override;


    //
    // Getters and setters
    //

public:

    long getNr() { return nr; }

    // Drive type
    DriveType getType() { return config.type; }
    void setType(DriveType t);

    // Drive speed
    i16 getSpeed() { return config.speed; }
    void setSpeed(i16 value);
    bool isOriginal() { return config.speed == 1; }
    bool isTurbo() { return config.speed < 0; }

    // Identification mode
    bool idMode() { return !motor; }
    u32 getDriveId();


    //
    // Handling the drive status register flags
    //
    
    // Returns true if this drive is currently selected
    inline bool isSelected() { return (prb & (0b1000 << nr)) == 0; }
    
    // Returns true if this drive is pushing data onto the data lines
    bool isDataSource();

    u8 driveStatusFlags();
    

    //
    // Operating the drive
    //
    
    // Turns the drive motor on or off
    void setMotor(bool value);
    void switchMotorOn() { setMotor(true); }
    void switchMotorOff() { setMotor(false); }

    Cycle motorOnTime();
    Cycle motorOffTime();
    bool motorAtFullSpeed();
    bool motorStopped();
    bool motorSpeedingUp() { return motor && !motorAtFullSpeed(); }
    bool motorSlowingDown() { return !motor && !motorStopped(); }

    // Selects the active drive head (0 = lower, 1 = upper)
    void selectSide(int side);

    // Reads a value from the drive head and rotates the disk
    u8 readHead();
    u16 readHead16();
    
    // Writes a value to the drive head and rotates the disk
    void writeHead(u8 value);
    void writeHead16(u16 value);

    // Emulate a disk rotation (moves head to the next byte)
    void rotate();

    // Rotates the disk to the next sync mark
    void findSyncMark();

    //
    // Moving the drive head
    //

    // Moves the drive head (0 = inwards, 1 = outwards).
    void moveHead(int dir);

    // Records a cylinder change (needed for diskPollingMode() to work)
    void recordCylinder(u8 cylinder);

    /* Returns true if the drive is in disk polling mode
     * Disk polling mode is detected by analyzing the movement history that
     * has been recorded by recordCylinder()
     */
    bool pollsForDisk();

    
    //
    // Handling disks
    //

    bool hasDisk() { return disk != NULL; }
    bool hasModifiedDisk() { return disk ? disk->isModified() : false; }
    void setModifiedDisk(bool value) { if (disk) disk->setModified(value); }
    
    bool hasWriteEnabledDisk();
    bool hasWriteProtectedDisk();
    void setWriteProtection(bool value); 
    void toggleWriteProtection();
    
    void ejectDisk();
    void insertDisk(Disk *disk);
    
    u64 fnv();
    
    //
    // Delegation methods
    //
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(u8 oldValue, u8 newValue);
};

#endif
