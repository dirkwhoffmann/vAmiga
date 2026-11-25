// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MMU030.h"
#include "MMU040.h"

namespace MOIRA_NAMESPACE {

/** MMU types
 *
 * The MMU is part of the CPU. The type that is present is determined by the
 * CPU model: 68030 and 68040 have an MMU, the 68EC030, 68EC040, and 68LC040
 * variants are cut down accordingly.
 */
enum class MMUModel
{
    NONE,                   ///< The MMU is not emulated (MMU instructions cause a line F exception)
    M68030,                 ///< The MMU of the 68030
    M68040                  ///< The MMU of the 68040
};

/** Memory management unit
 *
 * This class is a wrapper that dispatches to the MMU implementation of the
 * effectively present MMU model. Features that only exist in one MMU type are
 * accessed via the MMU object of that type (e.g., mmu.mmu030.crp).
 */
class MMU {

    // The MMU that is effectively present (resolved by Moira from the CPU model
    // and the selected MMU model)
    MMUModel model = MMUModel::NONE;

public:

    /// The MMU of the 68030
    MMU030 mmu030;
    /// The MMU of the 68040
    MMU040 mmu040;


    //
    // Constructing
    //

public:

    MMU(Moira &ref) : mmu030(ref), mmu040(ref) { }

    /// Initializes all registers and invalidates the ATC
    void reset() { mmu030.reset(); mmu040.reset(); }


    //
    // Configuring
    //

public:

    MMUModel getModel() const { return model; }
    void setModel(MMUModel newModel) { model = newModel; }
    bool isPresent() const { return model != MMUModel::NONE; }

    /// Indicates whether address translation is active
    bool isEnabled() const;


    //
    // Translating
    //

public:

    /// Translates a logical address to a physical address. If 'ci' is provided,
    /// it is set if the access must bypass the on-chip caches.
    std::optional<u32> translate(u32 addr, u8 fc, bool write, bool *ci = nullptr);

    /// Translates an address without side effects (for debuggers). Returns the
    /// address unchanged if translation is disabled and std::nullopt if the
    /// access would cause a bus error.
    std::optional<u32> peek(u32 addr, u8 fc, bool write = false) const;
};

inline bool
MMU::isEnabled() const
{
    switch (model) {

        case MMUModel::M68030: return mmu030.isEnabled();
        case MMUModel::M68040: return mmu040.isEnabled();
        default:               return false;
    }
}

inline std::optional<u32>
MMU::peek(u32 addr, u8 fc, bool write) const
{
    switch (model) {

        case MMUModel::M68030: return mmu030.peek(addr, fc, write);
        case MMUModel::M68040: return mmu040.peek(addr, fc, write);
        default:               return addr;
    }
}

inline std::optional<u32>
MMU::translate(u32 addr, u8 fc, bool write, bool *ci)
{
    switch (model) {

        case MMUModel::M68030: return mmu030.translate(addr, fc, write, ci);
        case MMUModel::M68040: return mmu040.translate(addr, fc, write, ci);

        default:

            if (ci) *ci = false;
            return addr;
    }
}

}
