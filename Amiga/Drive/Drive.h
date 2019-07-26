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

#include "HardwareComponent.h"
#include "Disk.h"

class ADFFile;

class Drive : public HardwareComponent {
    
    friend class DiskController;
    
private:
    
    // Drive number (0 = df0, 1 = df1, 2 = df2, 3 = df3)
    long nr = 0;
    
    // The type of this drive
    DriveType type = DRIVE_35_DD;
    
    /* Acceleration factor of this drive
     * This value equals the number of words that get transfered into memory
     * during a single disk DMA cycle. This value must be 1 to emulate a real
     * Amiga. If it set to, e.g., 2, the drive loads twice as fast.
     * A negative value indicates a turbo drive for which the exect value of
     * the acceleration factor has no meaning.
     */
    uint16_t speed = 1;
    
    // Position of the currently transmitted identification bit
    uint8_t idCount;

    // Value of the currently transmitted identification bit
    bool idBit;

    
    /* The latched MTR bit (motor control bit)
     * Each drive latches the motor signal at the time it is selected (i.e.,
     * when the SELx line is pulled down to 0). The disk drive motor stays in
     * this state until the drive is selected again. This bit also controls the
     * activity light on the front of the disk drive.
     */
    // bool mtr;
    
    /* Indicates if the motor is running at full speed
     * On a real drive, it can take up to one half second (500ms) until the
     * drive runs at full speed. We don't emulate  accurate timing here and
     * set the variable to true once the drive motor is switched on.
     *
     * ENHANCEMENT: MAKE it A COMPUTED VALUE:
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
    uint8_t dsklen;
    
    // A copy of the PRB register of CIA B
    uint8_t prb;
    
    // The current drive head location
    struct {
        uint8_t side;
        uint8_t cylinder;
        uint16_t offset;
    } head;
    
    /* History buffer storing the most recently visited tracks.
     * The buffer is used to detect the polling head movements that are issued
     * by track disc device to detect a newly inserted disk.
     */
    uint64_t cylinderHistory;
    
public:
    
    // The currently inserted disk (NULL if the drive is empty)
    Disk *disk = NULL;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Drive(unsigned nr);


    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & type
        & speed;
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
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
     void _powerOn() override;
     void _powerOff() override;
     void _reset() override;
     void _ping() override;
     void _dump() override;
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS };
    size_t _save(uint8_t *buffer) override;
    
public:
    
    //
    // Getter and setter
    //
    
    // Returns the device number (0 = df0, 1 = df1, 2 = df2, 3 = df3).
    long getNr() { return nr; }
    
    // Returns the drive type.
    DriveType getType() { return type; }

    // Sets the drive type.
    void setType(DriveType t);
    
    // Returns the accleration factor.
    uint16_t getSpeed() { return speed; }

    // Sets the accleration factor.
    void setSpeed(uint16_t value);

    // Indicates whether this drive is an original Amiga drive.
    bool isOriginalDrive() { return speed == 1; }

    // Indicates whether this drive is a turbo drive.
    bool isTurboDrive() { return speed > 128; }

    // Indicates whether identification mode is enabled.
    // bool idMode() { return motorStopped(); }
    bool idMode() { return !motor; }

    /* Returns the drive identification code.
     * Each drive identifies itself by a 32 bit identification code that is
     * transmitted via the DRVRDY signal in a special identification mode. The
     * identification mode is activated by switching the drive motor on and
     * off.
     */
    uint32_t getDriveId();
    
    
    //
    // Handling the drive status register flags
    //
    
    // Returns true if this drive is currently selected
    inline bool isSelected() { return (prb & (0b1000 << nr)) == 0; }
    
    // Returns true if this drive is pushing data onto the data lines
    bool isDataSource();

    uint8_t driveStatusFlags();
    

    //
    // Operating the drive
    //
    
    // Turns the drive motor on or off.
    void setMotor(bool value);
    void switchMotorOn() { setMotor(true); }
    void switchMotorOff() { setMotor(false); }

    Cycle motorOnTime();
    Cycle motorOffTime();
    bool motorAtFullSpeed();
    bool motorStopped();
    bool motorSpeedingUp() { return motor && !motorAtFullSpeed(); }
    bool motorSlowingDown() { return !motor && !motorStopped(); }

    // Selects the active drive head (0 = lower, 1 = upper).
    void selectSide(int side);

    // Reads a value from the drive head and rotates the disk.
    uint8_t readHead();
    uint16_t readHead16();
    
    // Writes a value to the drive head and rotates the disk.
    void writeHead(uint8_t value);
    void writeHead16(uint16_t value);

    // Emulate a disk rotation (moves head to the next byte).
    void rotate();

    // Rotates the disk to the next sync mark.
    void findSyncMark();

    //
    // Moving the drive head
    //

    // Moves the drive head (0 = inwards, 1 = outwards).
    void moveHead(int dir);

    // Records a cylinder change (needed for diskPollingMode() to work)
    void recordCylinder(uint8_t cylinder);

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
    void insertDisk(ADFFile *file);
    
    
    //
    // Delegation methods
    //
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(uint8_t oldValue, uint8_t newValue);
};

#endif
