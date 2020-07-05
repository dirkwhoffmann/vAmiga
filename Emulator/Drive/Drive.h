// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DRIVE_H
#define _AMIGA_DRIVE_H

#include "AmigaComponent.h"
#include "Disk.h"

class Drive : public AmigaComponent {
    
    friend class DiskController;
        
    // This object emulates drive df<nr>
    const int nr;

    // Bookkeeping
    DriveConfig config;
    DriveInfo info;

    // Drive motor status (on or off)
    bool motor;
    
    // Time stamp indicating the the latest change of the motor status
    Cycle switchCycle;
    
    // Recorded motor speed at 'motorCycle' in percent
    double switchSpeed;
    
    // Position of the currently transmitted identification bit
    u8 idCount;

    // Value of the currently transmitted identification bit
    bool idBit;

    // Records when the head started to step to another cylinder
    Cycle stepCycle;
    
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
    DriveInfo getInfo() { return HardwareComponent::getInfo(info); }

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.type
        & config.speed
        & config.startDelay
        & config.stopDelay
        & config.stepDelay;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & motor
        & switchCycle
        & switchSpeed
        & idCount
        & idBit
        & stepCycle
        & dskchange
        & dsklen
        & prb
        & head.side
        & head.cylinder
        & head.offset
        & cylinderHistory;
    }

private:

    void _reset(bool hard) override;
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
    bool idMode();
    u32 getDriveId();

    // Operation
    u8 getCylinder() { return head.cylinder; }
    
    
    //
    // Handling the drive status register flags
    //
    
    // Returns true if this drive is currently selected
    inline bool isSelected() { return (prb & (0b1000 << nr)) == 0; }
    
    u8 driveStatusFlags();
    

    //
    // Operating the drive
    //
    
    // Returns true if this drive emulates mechanical delays
    bool emulateMechanics() { return !isTurbo(); }
    
    // Returns the current motor speed in percent
    double motorSpeed();

    // Turns the drive motor on or off
    bool getMotor() { return motor; }
    void setMotor(bool value);
    void switchMotorOn() { setMotor(true); }
    void switchMotorOff() { setMotor(false); }

    // Informs about the current drive motor state
    bool motorSpeedingUp();
    bool motorAtFullSpeed();
    bool motorSlowingDown();
    bool motorStopped();

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

    // Returns wheather the drive is ready to accept a stepping pulse
    bool readyToStep();
    
    // Moves the drive head (0 = inwards, 1 = outwards).
    void step(int dir);

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
