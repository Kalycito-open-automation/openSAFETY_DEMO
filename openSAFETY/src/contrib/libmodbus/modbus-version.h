/**
 * \file modbus-version.h
 *
 * \addtogroup EXTERNLIBS
 * \{
 * \addtogroup libmodbus LibModbus
 * \{
 * \details
 * \{
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * \}
 * \author Stéphane Raimbault <stephane.raimbault@gmail.com>
 * \version 3.01
 */

#ifndef _MODBUS_VERSION_H_
#define _MODBUS_VERSION_H_

/* The major version, (1, if %LIBMODBUS_VERSION is 1.2.3) */
#define LIBMODBUS_VERSION_MAJOR (3)

/* The minor version (2, if %LIBMODBUS_VERSION is 1.2.3) */
#define LIBMODBUS_VERSION_MINOR (0)

/* The micro version (3, if %LIBMODBUS_VERSION is 1.2.3) */
#define LIBMODBUS_VERSION_MICRO (1)

/* The full version, like 1.2.3 */
#define LIBMODBUS_VERSION        3.0.1

/* The full version, in string form (suited for string concatenation)
 */
#define LIBMODBUS_VERSION_STRING "3.0.1"

/* Numerically encoded version, like 0x010203 */
#define LIBMODBUS_VERSION_HEX ((LIBMODBUS_MAJOR_VERSION << 24) |        \
                               (LIBMODBUS_MINOR_VERSION << 16) |        \
                               (LIBMODBUS_MICRO_VERSION << 8))

/* Evaluates to True if the version is greater than @major, @minor and @micro
 */
#define LIBMODBUS_VERSION_CHECK(major,minor,micro)      \
    (LIBMODBUS_VERSION_MAJOR > (major) ||               \
     (LIBMODBUS_VERSION_MAJOR == (major) &&             \
      LIBMODBUS_VERSION_MINOR > (minor)) ||             \
     (LIBMODBUS_VERSION_MAJOR == (major) &&             \
      LIBMODBUS_VERSION_MINOR == (minor) &&             \
      LIBMODBUS_VERSION_MICRO >= (micro)))

#endif /* _MODBUS_VERSION_H_ */

/**
 * \}
 * \}
 */
