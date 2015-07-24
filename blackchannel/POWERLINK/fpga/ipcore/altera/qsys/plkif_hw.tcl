# -----------------------------------------------------------------------------
# plkif_hw.tcl
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

source ../components/tcl/tripleBuffer.tcl

# -----------------------------------------------------------------------------
# module
# -----------------------------------------------------------------------------
set_module_property NAME {plkif}
set_module_property DISPLAY_NAME {POWERLINK Interface}

set_module_property VERSION {0.1}
set_module_property GROUP {System}
set_module_property DESCRIPTION {Lightweight interface to the POWERLINK processor}
set_module_property AUTHOR {B&R}
set_module_property opaque_address_map false

# -----------------------------------------------------------------------------
# Callback functions
# -----------------------------------------------------------------------------
set_module_property COMPOSITION_CALLBACK    compose_callback
set_module_property VALIDATION_CALLBACK     validate_callback

# -----------------------------------------------------------------------------
# User variables
# -----------------------------------------------------------------------------

# Set ipcore names
set spiBridgeInst spi_bridge_0
set tbufInst tripleBuffer_0
set tbufBridgePortA tbufPortA
set tbufBridgePortB tbufPortB

# Set id to each demo
set ID_DEMO_CN_GPIO 0
set ID_DEMO_SN_GPIO 1
set ID_DEMO_CUSTOM 2

# Set id to each connection
set ID_CON_SPI 0
set ID_CON_PAR 1
set ID_CON_AVALON 2

# Assemble settings list
set gTbufDemoPro { { 4 12 4 12} \
                   { 4 32 36 12 } \
                   { 4 }      \
};
set gTbufDemoCon { { 12 12 8 } \
                   { 12 36 36 } \
                   { 12 }      \
};

# Set name of GUI groups
set grpBufCfg "Internal Memory:"
set grpBuffTbuf "Triple buffer:"

set grpConCfg "Connection Interface:"
set grpConSpi "SPI Interface:"
set grpConPar "Parallel Interface:"
set grpConAvalon "Avalon Interface:"

# -----------------------------------------------------------------------------
# GUI parameters triple buffer
# -----------------------------------------------------------------------------
add_parameter gDemoCfg STRING "sn-gpio"
set_parameter_property gDemoCfg DEFAULT_VALUE "sn-gpio"
set_parameter_property gDemoCfg TYPE STRING
set_parameter_property gDemoCfg DISPLAY_NAME "Triple buffer layout"
set_parameter_property gDemoCfg ALLOWED_RANGES {"sn-gpio" "custom"}

add_display_item "" $grpBufCfg GROUP TAB
add_display_item $grpBufCfg $grpBuffTbuf GROUP

add_display_item $grpBuffTbuf gDemoCfg PARAMETER

source ../components/tcl/tripleBufferGui.tcl

add_display_item $grpBuffTbuf "Consumer Buffers" GROUP
add_display_item $grpBuffTbuf "Producer Buffers" GROUP

# -----------------------------------------------------------------------------
# GUI configuration connection
# -----------------------------------------------------------------------------
add_parameter gConCfg STRING "spi"
set_parameter_property gConCfg DEFAULT_VALUE "spi"
set_parameter_property gConCfg TYPE STRING
set_parameter_property gConCfg DISPLAY_NAME "POWERLINK Interface"
set_parameter_property gConCfg ALLOWED_RANGES {"spi" "parallel" "avalon"}

add_display_item "" $grpConCfg GROUP TAB
add_display_item $grpConCfg gConCfg PARAMETER

# -----------------------------------------------------------------------------
# GUI configuration SPI
# -----------------------------------------------------------------------------
source ../components/tcl/spiBridgeGui.tcl

add_display_item $grpConCfg $grpConSpi GROUP
add_display_item ${grpConSpi} gui_cpol PARAMETER
add_display_item ${grpConSpi} gui_cpha PARAMETER
add_display_item ${grpConSpi} gui_shiftdir PARAMETER
add_display_item ${grpConSpi} gui_regsize PARAMETER

# -----------------------------------------------------------------------------
# GUI configuration PAR
# -----------------------------------------------------------------------------
add_display_item $grpConCfg $grpConPar GROUP

# -----------------------------------------------------------------------------
# GUI configuration AVALON
# -----------------------------------------------------------------------------
add_display_item $grpConCfg $grpConAvalon GROUP


# -----------------------------------------------------------------------------
# System Info parameters
# -----------------------------------------------------------------------------

add_instance clk_50 clock_source 13.0
set_instance_parameter_value clk_50 clockFrequency {50000000.0}
set_instance_parameter_value clk_50 clockFrequencyKnown {1}
set_instance_parameter_value clk_50 resetSynchronousEdges {NONE}

add_instance ${tbufInst} tripleBuffer 0.0.1

add_instance ${tbufBridgePortB} altera_avalon_mm_bridge 13.0
set_instance_parameter_value ${tbufBridgePortB} DATA_WIDTH {32}
set_instance_parameter_value ${tbufBridgePortB} SYMBOL_WIDTH {8}
set_instance_parameter_value ${tbufBridgePortB} ADDRESS_WIDTH {10}
set_instance_parameter_value ${tbufBridgePortB} ADDRESS_UNITS {SYMBOLS}
set_instance_parameter_value ${tbufBridgePortB} MAX_BURST_SIZE {1}
set_instance_parameter_value ${tbufBridgePortB} MAX_PENDING_RESPONSES {1}
set_instance_parameter_value ${tbufBridgePortB} LINEWRAPBURSTS {0}
set_instance_parameter_value ${tbufBridgePortB} PIPELINE_COMMAND {0}
set_instance_parameter_value ${tbufBridgePortB} PIPELINE_RESPONSE {0}

# -----------------------------------------------------------------------------
# callbacks
# -----------------------------------------------------------------------------
proc validate_callback {} {
    set demoId [ getDemoId ]
    set conId [ getConnectionId ]

    if { $demoId == $::ID_DEMO_CUSTOM } {
        set_parameter_property "numOfCon" VISIBLE true
        set_parameter_property "numOfPro" VISIBLE true

        set conMaxCnt [get_parameter_value "numOfCon" ]
        setGuiParamListVis $::glb_listCon $conMaxCnt

        set proMaxCnt [get_parameter_value "numOfPro" ]
        setGuiParamListVis $::glb_listPro $proMaxCnt
    } else {
        set_parameter_property "numOfCon" VISIBLE false
        set_parameter_property "numOfPro" VISIBLE false

        setGuiParamListVis $::glb_listCon 0
        setGuiParamListVis $::glb_listPro 0
    }

    # Handle connection GUI parameters
    setConnectionGuiVisibility $conId
}

proc compose_callback {} {
    set demoId [ getDemoId ]
    set conId [ getConnectionId ]

    if { $demoId == $::ID_DEMO_CUSTOM} {
        # Forward GUI settings to local structure
        updateDemoSettings $demoId
    }

    # Update triple buffer ipcore from gui settings
    setTbufInstBufferSizes $::tbufInst $demoId

    # Handle connection settings
    if { $conId == $::ID_CON_SPI } {
        handleSpiInterface $demoId $::tbufInst $::spiBridgeInst
    } elseif { $conId == $::ID_CON_AVALON } {
        handleAvalonInterface $demoId $::tbufInst $::tbufBridgePortA
    } else {
        send_message ERROR "Parallel interface not supported in this version!"
    }
}

# -----------------------------------------------------------------------------
# internal functions
# -----------------------------------------------------------------------------

proc setConnectionGuiVisibility { conId } {
    # Handle connection parameters
    if { $conId == $::ID_CON_SPI } {
        set_display_item_property $::grpConSpi VISIBLE true
    } elseif { $conId == $::ID_CON_PAR } {
        set_display_item_property $::grpConSpi VISIBLE false
    } elseif { $conId == $::ID_CON_AVALON } {
        set_display_item_property $::grpConSpi VISIBLE false
    } else {
        send_message ERROR "Invalid connection!"
    }
}

# Return connection id from string
proc getConnectionId { } {
    set selConn [ get_parameter_value gConCfg ]

    switch $selConn {
       "spi" {
              return $::ID_CON_SPI
       }
       "parallel" {
              return $::ID_CON_PAR
       }
       "avalon" {
              return $::ID_CON_AVALON
       }
       default {
              send_message ERROR "Invalid connection selected!"
       }
    }
}

# Return demo id from string
proc getDemoId { } {
    set selDemo [ get_parameter_value gDemoCfg ]

    switch $selDemo {
       "cn-gpio" {
              return $::ID_DEMO_CN_GPIO
       }
       "sn-gpio" {
              return $::ID_DEMO_SN_GPIO
       }
       "custom" {
              return $::ID_DEMO_CUSTOM
       }
       default {
              send_message ERROR "Invalid demo selected!"
       }
    }
}

# Forward size of buffers to triple buffer ipcore
proc setTbufInstBufferSizes { tbufInstance demoId} {
    set demoList [ lindex $::gTbufDemoCon $demoId ]
    set numCon [llength [lindex $demoList ]]
    set_instance_parameter_value ${tbufInstance} numOfCon $numCon

    for {set i 0} {$i < $numCon } {incr i} {
        set_instance_parameter_value ${tbufInstance} conBufSize${i} [ lindex $demoList $i ]
    }

    set demoList [ lindex $::gTbufDemoPro $demoId ]
    set numPro [llength [lindex $demoList ]]
    set_instance_parameter_value ${tbufInstance} numOfPro $numPro

    for {set i 0} {$i < $numPro } {incr i} {
        set_instance_parameter_value ${tbufInstance} proBufSize${i} [ lindex $demoList $i ]
    }
}

# Forward gui settings to local structure
proc updateDemoSettings { demoId } {
    set demoList { }
    set numOfCon [ get_parameter_value numOfCon ]

    for {set i 0} {$i < $::CON_MAX_BUF } {incr i} {
        if { $i < $numOfCon } {
            set conGuiVal [ get_parameter_value ${::CON_PARAM}${i} ]
            lappend demoList $conGuiVal
        }
    }

    set ::gTbufDemoCon [ lreplace $::gTbufDemoCon $demoId $demoId $demoList ]

    # Handle producing buffers
    set demoList { }
    set numOfPro [ get_parameter_value numOfPro ]

    for {set i 0} {$i < $::PRO_MAX_BUF } {incr i} {
        if { $i < $numOfPro } {
            set proGuiVal [ get_parameter_value ${::PRO_PARAM}${i} ]
            lappend demoList $proGuiVal
        }
    }

    set ::gTbufDemoPro [ lreplace $::gTbufDemoPro $demoId $demoId $demoList ]
}

# SPI Interface selected -> Instantiate SPI bridge core and set parameters
proc handleSpiInterface { demoId tbufInstance spiInstance} {
    set ackRegSize 4

    # Setup SPI bridge ipcore
    add_instance ${spiInstance} spi_bridge 0.0.1

    set_instance_parameter_value ${spiInstance} gui_cpol [ get_parameter_value "gui_cpol"]
    set_instance_parameter_value ${spiInstance} gui_cpha [ get_parameter_value "gui_cpha"]
    set_instance_parameter_value ${spiInstance} gui_shiftdir [ get_parameter_value "gui_shiftdir"]
    set_instance_parameter_value ${spiInstance} gui_regsize [ get_parameter_value "gui_regsize"]

    # Get total size of consumer (+ ack reg)
    set sizeCon $ackRegSize
    set buffCon [ lindex $::gTbufDemoCon $demoId ]
    foreach buff $buffCon {
        set sizeCon [expr $sizeCon + $buff]
    }

    # Get total size of producer (+ ack reg)
    set sizePro $ackRegSize
    set buffPro [ lindex $::gTbufDemoPro $demoId ]
    foreach buff $buffPro {
        set sizePro [expr $sizePro + $buff]
    }

    set_instance_parameter_value ${spiInstance} gui_readsize $sizeCon
    set_instance_parameter_value ${spiInstance} gui_writesize $sizePro
    set_instance_parameter_value ${spiInstance} gui_enconmem 1

    # handle SPI bridge connections
    add_connection ${spiInstance}.bridge ${tbufInstance}.porta avalon
    set_connection_parameter_value ${spiInstance}.bridge/${tbufInstance}.porta arbitrationPriority {1}
    set_connection_parameter_value ${spiInstance}.bridge/${tbufInstance}.porta baseAddress {0x0000}
    set_connection_parameter_value ${spiInstance}.bridge/${tbufInstance}.porta defaultConnection {0}

    add_connection clk_50.clk ${spiInstance}.c0 clock
    add_connection clk_50.clk_reset ${spiInstance}.r0 reset

    # Export SPI bridge interface
    add_interface ${spiInstance}_spi conduit end
    set_interface_property ${spiInstance}_spi EXPORT_OF ${spiInstance}.spi

    # Set triple buffer streaming option
    set_instance_parameter_value ${tbufInstance} gui_enableStream 1
}

# Avalon interface selected -> Instantiate avalon bridge core and export interface
proc handleAvalonInterface { demoId tbufInstance tbufBridgePortA } {

    # Instantiate bridge for port A
    add_instance ${tbufBridgePortA} altera_avalon_mm_bridge 13.0
    set_instance_parameter_value ${tbufBridgePortA} DATA_WIDTH {32}
    set_instance_parameter_value ${tbufBridgePortA} SYMBOL_WIDTH {8}
    set_instance_parameter_value ${tbufBridgePortA} ADDRESS_WIDTH {10}
    set_instance_parameter_value ${tbufBridgePortA} ADDRESS_UNITS {SYMBOLS}
    set_instance_parameter_value ${tbufBridgePortA} MAX_BURST_SIZE {1}
    set_instance_parameter_value ${tbufBridgePortA} MAX_PENDING_RESPONSES {1}
    set_instance_parameter_value ${tbufBridgePortA} LINEWRAPBURSTS {0}
    set_instance_parameter_value ${tbufBridgePortA} PIPELINE_COMMAND {0}
    set_instance_parameter_value ${tbufBridgePortA} PIPELINE_RESPONSE {0}

    # Connect port A bridge
    add_connection ${tbufBridgePortA}.m0 ${tbufInstance}.porta avalon
    set_connection_parameter_value ${tbufBridgePortA}.m0/${tbufInstance}.porta arbitrationPriority {1}
    set_connection_parameter_value ${tbufBridgePortA}.m0/${tbufInstance}.porta baseAddress {0x0000}
    set_connection_parameter_value ${tbufBridgePortA}.m0/${tbufInstance}.porta defaultConnection {0}

    add_connection clk_50.clk ${tbufBridgePortA}.clk clock
    add_connection clk_50.clk_reset ${tbufBridgePortA}.reset reset

    add_interface ${tbufBridgePortA}_s0 avalon slave
    set_interface_property ${tbufBridgePortA}_s0 EXPORT_OF ${tbufBridgePortA}.s0

    # Reset triple buffer streaming option
    set_instance_parameter_value ${tbufInstance} gui_enableStream 0

}

# -----------------------------------------------------------------------------
# connection points
# -----------------------------------------------------------------------------
add_connection ${tbufBridgePortB}.m0 ${tbufInst}.portb avalon
set_connection_parameter_value ${tbufBridgePortB}.m0/${tbufInst}.portb arbitrationPriority {1}
set_connection_parameter_value ${tbufBridgePortB}.m0/${tbufInst}.portb baseAddress {0x0000}
set_connection_parameter_value ${tbufBridgePortB}.m0/${tbufInst}.portb defaultConnection {0}

add_connection clk_50.clk ${tbufInst}.c0 clock
add_connection clk_50.clk_reset ${tbufInst}.r0 reset

add_connection clk_50.clk ${tbufBridgePortB}.clk clock
add_connection clk_50.clk_reset ${tbufBridgePortB}.reset reset


# exported interfaces
add_interface clk clock sink
set_interface_property clk EXPORT_OF clk_50.clk_in
add_interface reset reset sink
set_interface_property reset EXPORT_OF clk_50.clk_in_reset
add_interface ${tbufBridgePortB}_s0 avalon slave
set_interface_property ${tbufBridgePortB}_s0 EXPORT_OF ${tbufBridgePortB}.s0
