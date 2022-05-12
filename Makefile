
DEVICE    := Arria10
TARGETS   := emu.exe fpga.exe fpga_ghdl.exe fpga_qii.exe
TESTBENCH := HLS_DataFlow_library
COMPONENT := test_comp
            
CXX      := i++
RM     := rm -rfv
CXXFLAGS := -lboost_unit_test_framework
#TOOLCHAIN := --gcc-toolchain=/opt/intelFPGA_pro/21.2/gcc

.PHONY: test
test: $(TARGETS)
	@$(foreach t,$(TARGETS),echo ./$(t); ./$(t); echo "";)

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	-$(RM) $(TARGETS) *.out *.exe *.o *.a *.prj $(COMPONENT)_emu $(COMPONENT)_fpga $(COMPONENT)_fpga_ghdl $(COMPONENT)_fpga_qii 
	rm -f result/*.dat

$(COMPONENT)_%.o : $(COMPONENT).cpp
	$(CXX) -march=$(ARCH) $(TOOLCHAIN) -g -Wno-return-type-c-linkage -I ./lib -c $< -o $@

$(COMPONENT)_% : $(COMPONENT)_%.o
	touch $@
	$(CXX) -march=$(ARCH) -g --fpga-only $(GHDL) $(QUARTUSCOMPILE) -I ./lib $<

$(TESTBENCH)_%.o : $(TESTBENCH).cpp ./lib/*.hpp *.hpp
	$(CXX) $(TOOLCHAIN) -g -Wno-return-type-c-linkage -I ./lib -I ./tool -c $< -o $@

emu.exe: ARCH=x86-64
emu.exe: $(COMPONENT)_emu.o $(COMPONENT)_emu $(TESTBENCH)_emu.o
	$(CXX) $(TOOLCHAIN) -g -lboost_unit_test_framework $(COMPONENT)_emu.o $(TESTBENCH)_emu.o -o $@

fpga.exe: ARCH=$(DEVICE)
fpga.exe: $(COMPONENT)_fpga.o $(COMPONENT)_fpga $(TESTBENCH)_fpga.o
	$(CXX) $(TOOLCHAIN) -g --x86-only -lboost_unit_test_framework $(COMPONENT)_fpga.o $(TESTBENCH)_fpga.o -o $@

fpga_ghdl.exe: ARCH=$(DEVICE)
fpga_ghdl.exe: GHDL=-ghdl
fpga_ghdl.exe: $(COMPONENT)_fpga_ghdl.o $(COMPONENT)_fpga_ghdl $(TESTBENCH)_fpga_ghdl.o
	$(CXX) $(TOOLCHAIN) -g --x86-only -lboost_unit_test_framework $(COMPONENT)_fpga_ghdl.o $(TESTBENCH)_fpga_ghdl.o -o $@

fpga_qii.exe: ARCH=$(DEVICE)
fpga_qii.exe: GHDL=-ghdl
fpga_qii.exe: QUARTUSCOMPILE=--quartus-compile
fpga_qii.exe: $(COMPONENT)_fpga_qii.o $(COMPONENT)_fpga_qii $(TESTBENCH)_fpga_qii.o
	$(CXX) $(TOOLCHAIN) -g --x86-only -lboost_unit_test_framework $(COMPONENT)_fpga_qii.o $(TESTBENCH)_fpga_qii.o -o $@

mytest: HLS_DataFlow_testbench.cpp
	$(CXX) -I doctest/doctest -c HLS_DataFlow_testbench.cpp -o HLS_DataFlow_testbench.exe
