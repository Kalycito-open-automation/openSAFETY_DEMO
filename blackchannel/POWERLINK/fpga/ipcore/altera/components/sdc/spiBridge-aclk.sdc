###############################################################################
# SPI Bridge Timing Constraints for
#  Asynchronous Clock Architecture
###############################################################################

###############################################################################
# I/O MIN/MAX DELAY [ns]
set spi_in_max      10.0
set spi_in_min      5.0
set spi_out_max     10.0
set spi_out_min     5.0

###############################################################################
# SPI CLOCK RATE
set spi_clk_rate    25MHz

###############################################################################
# HIERARCHY
set instSpiBridge   *alteraSpiBridge
set instSpiCore     $instSpiBridge*spiSlave

###############################################################################
# REGISTERS

## Capture register
set reg_spiCap [get_registers ${instSpiCore}*spiCap]

## Output register is the last shift register ff
    set instSpiReg "${instSpiCore}*spiReg"
    set spiRegCnt 0
    foreach_in_collection reg [get_registers ${instSpiReg}[?]] {
        incr spiRegCnt
    }
    set spiRegCnt [expr $spiRegCnt - 1]
set reg_spiOut [get_registers "${instSpiReg}[${spiRegCnt}]"]

###############################################################################
# PINS

## Spi select
set pin_sel [get_pins -hierarchical *spiCap|ena]

###############################################################################
# CLK

## Get driving clock of spi capture register, which is the spi clk...
set fanins [get_fanins $reg_spiCap -clock]

## Create for every found fanin clock a clock
foreach_in_collection fanin_keeper $fanins {
    set clk [get_node_info $fanin_keeper -name]
    create_clock -period $spi_clk_rate -name spi_clk [get_ports $clk]
}

## Create virtual clock
create_generated_clock -source spi_clk -name spi_clk_virt

###############################################################################
# SETUP / HOLD Timing for inputs

## Get port name of capture register
foreach_in_collection fanin [get_fanins $reg_spiCap -synch] {
    set port_mosi [get_node_info $fanin -name]
}

## MOSI
set_input_delay -clock spi_clk_virt -max $spi_in_max [get_ports $port_mosi]
set_input_delay -clock spi_clk_virt -min $spi_in_min [get_ports $port_mosi]

###############################################################################
# CLOCK TO OUTPUT Timing for outputs

## Get port name of output register
foreach_in_collection fanout [get_fanouts $reg_spiOut] {
    set port_miso [get_node_info $fanout -name]
}

## MISO
set_output_delay -clock spi_clk_virt -max $spi_out_max [get_ports $port_miso]
set_output_delay -clock spi_clk_virt -min $spi_out_min [get_ports $port_miso]

###############################################################################
# TIMING IGNORE

## nSEL
set_false_path -from [get_ports *] -to ${pin_sel}

###############################################################################
# CLOCK GROUPS
set_clock_groups -asynchronous -group {spi_clk spi_clk_virt}
