################################################################################
#
# CMake file of application interface on pcp for PIFA
#
# Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holders nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
################################################################################

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)

PROJECT( pcpappif )

SET( PLK_STACK_DIR ${CMAKE_SOURCE_DIR}/stacks/openPOWERLINK )
SET( IP_STACK_DIR ${CMAKE_SOURCE_DIR}/stacks/ip )

########################################################################
# Set all source files
########################################################################
SET ( APPIF_SRCS
    ${PROJECT_SOURCE_DIR}/appif.c
    ${PROJECT_SOURCE_DIR}/main.c
    ${PROJECT_SOURCE_DIR}/tbuf.c
    ${PROJECT_SOURCE_DIR}/status.c
    ${PROJECT_SOURCE_DIR}/cc.c
    ${PROJECT_SOURCE_DIR}/icc.c
    ${PROJECT_SOURCE_DIR}/occ.c
    ${PROJECT_SOURCE_DIR}/pdo.c
    ${PROJECT_SOURCE_DIR}/rpdo.c
    ${PROJECT_SOURCE_DIR}/tpdo.c
    ${PROJECT_SOURCE_DIR}/ssdo.c
    ${PROJECT_SOURCE_DIR}/fifo.c
    ${appifcommon_SOURCE_DIR}/timeout.c
    ${appifcommon_SOURCE_DIR}/ccobject.c
)

SET ( IPSTACK_SRCS
    ${IP_STACK_DIR}/ip.c
    ${IP_STACK_DIR}/ip_name.c
    ${IP_STACK_DIR}/ip_dhcp.c
    ${IP_STACK_DIR}/edrv2veth.c
    ${IP_STACK_DIR}/hton.c
)

SET ( PLKSTACK_SRCS
    ${PLK_STACK_DIR}/examples/demo_cn_embedded/event-cn.c
    ${PLK_STACK_DIR}/libs/ami/amiarm.c
    ${PLK_STACK_DIR}/libs/omethlib/omethlib.c
    ${PLK_STACK_DIR}/libs/omethlib/omethlibint.c
    ${PLK_STACK_DIR}/libs/omethlib/omethlib_phycfg_generic.c
    ${PLK_STACK_DIR}/libs/circbuf/circbuffer.c
    ${PLK_STACK_DIR}/libs/circbuf/circbuf-noos.c
    ${PLK_STACK_DIR}/libs/trace/trace-printf.c
    ${PLK_STACK_DIR}/stack/src/arch/altera_nios2/target-nios2.c
    ${PLK_STACK_DIR}/stack/src/arch/altera_nios2/lock-localnoos.c
    ${PLK_STACK_DIR}/stack/src/common/debug.c
    ${PLK_STACK_DIR}/stack/src/common/errstring.c
    ${PLK_STACK_DIR}/stack/src/common/dll/dllcal-direct.c
    ${PLK_STACK_DIR}/stack/src/common/event/event.c
    ${PLK_STACK_DIR}/stack/src/common/timer/timer-generic.c
    ${PLK_STACK_DIR}/stack/src/kernel/ctrl/ctrlk.c
    ${PLK_STACK_DIR}/stack/src/kernel/ctrl/ctrlkcal-direct.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllk.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllkfilter.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllkstatemachine.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllkevent.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllkframe.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllkcal.c
    ${PLK_STACK_DIR}/stack/src/kernel/dll/dllkcal-circbuf.c
    ${PLK_STACK_DIR}/stack/src/kernel/edrv/edrv-openmac.c
    ${PLK_STACK_DIR}/stack/src/kernel/errhnd/errhndk.c
    ${PLK_STACK_DIR}/stack/src/kernel/errhnd/errhndkcal-local.c
    ${PLK_STACK_DIR}/stack/src/kernel/event/eventk.c
    ${PLK_STACK_DIR}/stack/src/kernel/event/eventkcalintf-circbuf.c
    ${PLK_STACK_DIR}/stack/src/kernel/event/eventkcal-nooscircbuf.c
    ${PLK_STACK_DIR}/stack/src/kernel/hrtimer/hrtimersync-openmac.c
    ${PLK_STACK_DIR}/stack/src/kernel/nmt/nmtk.c
    ${PLK_STACK_DIR}/stack/src/kernel/pdo/pdok.c
    ${PLK_STACK_DIR}/stack/src/kernel/pdo/pdokcal.c
    ${PLK_STACK_DIR}/stack/src/kernel/pdo/pdokcalmem-local.c
    ${PLK_STACK_DIR}/stack/src/kernel/pdo/pdokcalsync-null.c
    ${PLK_STACK_DIR}/stack/src/kernel/pdo/pdokcal-triplebufshm.c
    ${PLK_STACK_DIR}/stack/src/kernel/timestamp/timestamp-openmac.c
    ${PLK_STACK_DIR}/stack/src/user/api/generic.c
    ${PLK_STACK_DIR}/stack/src/user/ctrl/ctrlu.c
    ${PLK_STACK_DIR}/stack/src/user/ctrl/ctrlucal-direct.c
    ${PLK_STACK_DIR}/stack/src/user/dll/dllucal.c
    ${PLK_STACK_DIR}/stack/src/user/dll/dllucal-circbuf.c
    ${PLK_STACK_DIR}/stack/src/user/errhnd/errhndu.c
    ${PLK_STACK_DIR}/stack/src/user/errhnd/errhnducal-local.c
    ${PLK_STACK_DIR}/stack/src/user/event/eventu.c
    ${PLK_STACK_DIR}/stack/src/user/event/eventucalintf-circbuf.c
    ${PLK_STACK_DIR}/stack/src/user/event/eventucal-nooscircbuf.c
    ${PLK_STACK_DIR}/stack/src/user/nmt/nmtu.c
    ${PLK_STACK_DIR}/stack/src/user/nmt/nmtcnu.c
    ${PLK_STACK_DIR}/stack/src/user/obd/obd.c
    ${PLK_STACK_DIR}/stack/src/user/obd/obdcreate.c
    ${PLK_STACK_DIR}/stack/src/user/pdo/pdou.c
    ${PLK_STACK_DIR}/stack/src/user/pdo/pdoucal.c
    ${PLK_STACK_DIR}/stack/src/user/pdo/pdoucalmem-local.c
    ${PLK_STACK_DIR}/stack/src/user/pdo/pdoucalsync-null.c
    ${PLK_STACK_DIR}/stack/src/user/pdo/pdoucal-triplebufshm.c
    ${PLK_STACK_DIR}/stack/src/user/sdo/sdo-asndu.c
    ${PLK_STACK_DIR}/stack/src/user/sdo/sdo-asysequ.c
    ${PLK_STACK_DIR}/stack/src/user/sdo/sdo-comu.c
    ${PLK_STACK_DIR}/stack/src/user/veth/veth-noos.c
    ${PLK_STACK_DIR}/stack/src/user/sdo/sdoudp-noos.c
    ${PLK_STACK_DIR}/stack/src/user/ledu.c
)

########################################################################
# Set include paths
########################################################################
SET ( APPIF_INCS
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/target/altera/include
    ${appifcommon_SOURCE_DIR}/include
    ${DEMO_CONFIG_DIR}/config/objdict
    ${DEMO_CONFIG_DIR}/config/tbuf/include
    ${IP_STACK_DIR}
    ${PLK_STACK_DIR}/examples/demo_cn_embedded
    ${PLK_STACK_DIR}/include
    ${PLK_STACK_DIR}/objdicts/generic
    ${PLK_STACK_DIR}/libs/omethlib
    ${PLK_STACK_DIR}/libs/circbuf
    ${PLK_STACK_DIR}/stack/include/target/openmac
    ${PLK_STACK_DIR}/stack/src/arch/altera_nios2
    ${PLK_STACK_DIR}/stack/src
)

IF ( CFG_GEN_NIOS_TARGET )
    INCLUDE (nios2.cmake)
ENDIF ( CFG_GEN_NIOS_TARGET )

