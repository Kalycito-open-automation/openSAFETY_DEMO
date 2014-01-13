# -----------------------------------------------------------------------------
# plkif-config.tcl
# -----------------------------------------------------------------------------
#
#    (c) B&R, 2013
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions
#    are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#    3. Neither the name of B&R nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without prior written permission. For written
#       permission, please contact office@br-automation.com
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#    COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#    POSSIBILITY OF SUCH DAMAGE.
#
# -----------------------------------------------------------------------------
package require -exact qsys 13.0

# -----------------------------------------------------------------------------
# Constant settings
# -----------------------------------------------------------------------------
set instPlkifName "plkif_0"

# -----------------------------------------------------------------------------
# Input parameters
# -----------------------------------------------------------------------------
if { [ info exists selDemo ] != 1 } {
    send_message ERROR "Invalid input parameter!"
    exit 1;
} else {
    set selectedDemo $selDemo
}

if { [ info exists qsysSystemName ] != 1 } {
    send_message ERROR "Invalid qsys toplevel file!"
    exit 1;
} else {
    set qsysTopName $qsysSystemName
}

# -----------------------------------------------------------------------------
# Search for POWERLINK interface instance
# -----------------------------------------------------------------------------
set instances [ get_instances ]
set plkifHere 0

foreach instance $instances {
    if { $instPlkifName == $instance} {
        set plkifHere 1;
    }
}

if {$plkifHere != 1} {
    send_message ERROR "POWERLINK interface ipcore was not found!"
    exit 1;
}

send_message INFO "POWERLINK interface ipcore found!"

# -----------------------------------------------------------------------------
# Adapt demo configuration variable
# -----------------------------------------------------------------------------
set currentDemo [ get_instance_parameter_value $instPlkifName gDemoCfg ]

if { $currentDemo != $selectedDemo } {
    #set_instance_parameter_value $instPlkifName gDemoCfg $selectedDemo
    send_message INFO "Current demo set to demo '$selectedDemo'! Rebuild FPGA design with '$instPlkifName' set to '$selectedDemo'!"
    exit 2;
} else {
    send_message INFO "Selected demo was already set to demo '$selectedDemo'! No rebuild of FPGA design necessary!"
}

#save_system "$qsysTopName.qsys"
exit 0; # Success!