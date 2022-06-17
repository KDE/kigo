# Find the GNU Go executable
#
# Defines the following variables
#  GNUGo_EXECUTABLE - path of the GNU Go executable

#=============================================================================
# SPDX-FileCopyrightText: 2022 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(GNUGo_EXECUTABLE NAMES gnugo)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GNUGo DEFAULT_MSG GNUGo_EXECUTABLE)

mark_as_advanced(GNUGo_EXECUTABLE)
