// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SequencerTypes.h"
#include "SubComponent.h"

class Sequencer : public SubComponent
{
    //
    // Initializing
    //

    public:

    Sequencer(Amiga& ref);


    //
    // Methods from AmigaObject
    //

    private:

    const char *getDescription() const override { return "Sequencer"; }
    void _dump(dump::Category category, std::ostream& os) const override;


    //
    // Methods from AmigaComponent
    //

    private:

    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {

    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

};
