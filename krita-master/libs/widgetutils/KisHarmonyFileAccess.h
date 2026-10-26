/*
 * SPDX-FileCopyrightText: 2026 Krita contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HARMONY_FILE_ACCESS_H
#define KIS_HARMONY_FILE_ACCESS_H

#include "kritawidgetutils_export.h"

class QString;

namespace KisHarmonyFileAccess
{
/** Activate a permission persisted by an earlier process. No-op off HarmonyOS. */
KRITAWIDGETUTILS_EXPORT void activate(const QString &path, bool writable = true);

/** Persist the picker permission for a document that must survive a restart. */
KRITAWIDGETUTILS_EXPORT void persist(const QString &path, bool writable = true);
}

#endif
