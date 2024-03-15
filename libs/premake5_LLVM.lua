
local LLVM_libs_loc = (config.location .. "/libs/LLVM/lib/")


-- TODO: go through and make sure these files are correct / organize
function link_llvm_old()
	links{
		-- Don't use (breaks interpreter)
		-- (LLVM_libs_loc .. "/LLVM-C"),

		-- (LLVM_libs_loc .. "/LLVMAArch64AsmParser"),
		-- (LLVM_libs_loc .. "/LLVMAArch64CodeGen"),
		-- (LLVM_libs_loc .. "/LLVMAArch64Desc"),
		-- (LLVM_libs_loc .. "/LLVMAArch64Disassembler"),
		-- (LLVM_libs_loc .. "/LLVMAArch64Info"),
		-- (LLVM_libs_loc .. "/LLVMAArch64Utils"),
		(LLVM_libs_loc .. "/LLVMAggressiveInstCombine"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUDesc"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUInfo"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUTargetMCA"),
		-- (LLVM_libs_loc .. "/LLVMAMDGPUUtils"),
		(LLVM_libs_loc .. "/LLVMAnalysis"),
		-- (LLVM_libs_loc .. "/LLVMARMAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMARMCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMARMDesc"),
		-- (LLVM_libs_loc .. "/LLVMARMDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMARMInfo"),
		-- (LLVM_libs_loc .. "/LLVMARMUtils"),
		(LLVM_libs_loc .. "/LLVMAsmParser"),
		(LLVM_libs_loc .. "/LLVMAsmPrinter"),
		-- (LLVM_libs_loc .. "/LLVMAVRAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMAVRCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMAVRDesc"),
		-- (LLVM_libs_loc .. "/LLVMAVRDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMAVRInfo"),
		(LLVM_libs_loc .. "/LLVMBinaryFormat"),
		(LLVM_libs_loc .. "/LLVMBitReader"),
		(LLVM_libs_loc .. "/LLVMBitstreamReader"),
		(LLVM_libs_loc .. "/LLVMBitWriter"),
		-- (LLVM_libs_loc .. "/LLVMBPFAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMBPFCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMBPFDesc"),
		-- (LLVM_libs_loc .. "/LLVMBPFDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMBPFInfo"),
		(LLVM_libs_loc .. "/LLVMCFGuard"),
		(LLVM_libs_loc .. "/LLVMCFIVerify"),
		(LLVM_libs_loc .. "/LLVMCodeGen"),
		(LLVM_libs_loc .. "/LLVMCodeGenTypes"),
		(LLVM_libs_loc .. "/LLVMCore"),
		(LLVM_libs_loc .. "/LLVMCoroutines"),
		(LLVM_libs_loc .. "/LLVMCoverage"),
		(LLVM_libs_loc .. "/LLVMDebugInfoBTF"),
		(LLVM_libs_loc .. "/LLVMDebugInfoCodeView"),
		(LLVM_libs_loc .. "/LLVMDebuginfod"),
		(LLVM_libs_loc .. "/LLVMDebugInfoDWARF"),
		(LLVM_libs_loc .. "/LLVMDebugInfoGSYM"),
		(LLVM_libs_loc .. "/LLVMDebugInfoLogicalView"),
		(LLVM_libs_loc .. "/LLVMDebugInfoMSF"),
		(LLVM_libs_loc .. "/LLVMDebugInfoPDB"),
		(LLVM_libs_loc .. "/LLVMDemangle"),
		(LLVM_libs_loc .. "/LLVMDiff"),
		(LLVM_libs_loc .. "/LLVMDlltoolDriver"),
		-- (LLVM_libs_loc .. "/LLVMDWARFLinker"),
		-- (LLVM_libs_loc .. "/LLVMDWARFLinkerParallel"),
		(LLVM_libs_loc .. "/LLVMDWP"),
		(LLVM_libs_loc .. "/LLVMExecutionEngine"),
		-- (LLVM_libs_loc .. "/LLVMExegesis"),
		-- (LLVM_libs_loc .. "/LLVMExegesisAArch64"),
		-- (LLVM_libs_loc .. "/LLVMExegesisMips"),
		-- (LLVM_libs_loc .. "/LLVMExegesisPowerPC"),
		-- (LLVM_libs_loc .. "/LLVMExegesisX86"),
		(LLVM_libs_loc .. "/LLVMExtensions"),
		(LLVM_libs_loc .. "/LLVMFileCheck"),
		(LLVM_libs_loc .. "/LLVMFrontendHLSL"),
		(LLVM_libs_loc .. "/LLVMFrontendOpenACC"),
		(LLVM_libs_loc .. "/LLVMFrontendOpenMP"),
		(LLVM_libs_loc .. "/LLVMFuzzerCLI"),
		(LLVM_libs_loc .. "/LLVMFuzzMutate"),
		(LLVM_libs_loc .. "/LLVMGlobalISel"),
		-- (LLVM_libs_loc .. "/LLVMHexagonAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMHexagonCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMHexagonDesc"),
		-- (LLVM_libs_loc .. "/LLVMHexagonDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMHexagonInfo"),
		(LLVM_libs_loc .. "/LLVMInstCombine"),
		(LLVM_libs_loc .. "/LLVMInstrumentation"),
		(LLVM_libs_loc .. "/LLVMInterfaceStub"),
		(LLVM_libs_loc .. "/LLVMInterpreter"),
		-- (LLVM_libs_loc .. "/LLVMipo"),
		(LLVM_libs_loc .. "/LLVMIRPrinter"),
		(LLVM_libs_loc .. "/LLVMIRReader"),
		(LLVM_libs_loc .. "/LLVMJITLink"),
		-- (LLVM_libs_loc .. "/LLVMLanaiAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMLanaiCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMLanaiDesc"),
		-- (LLVM_libs_loc .. "/LLVMLanaiDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMLanaiInfo"),
		(LLVM_libs_loc .. "/LLVMLibDriver"),
		(LLVM_libs_loc .. "/LLVMLineEditor"),
		(LLVM_libs_loc .. "/LLVMLinker"),
		-- (LLVM_libs_loc .. "/LLVMLoongArchAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMLoongArchCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMLoongArchDesc"),
		-- (LLVM_libs_loc .. "/LLVMLoongArchDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMLoongArchInfo"),
		(LLVM_libs_loc .. "/LLVMLTO"),
		(LLVM_libs_loc .. "/LLVMMC"),
		(LLVM_libs_loc .. "/LLVMMCA"),
		(LLVM_libs_loc .. "/LLVMMCDisassembler"),
		(LLVM_libs_loc .. "/LLVMMCJIT"),
		(LLVM_libs_loc .. "/LLVMMCParser"),
		-- (LLVM_libs_loc .. "/LLVMMipsAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMMipsCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMMipsDesc"),
		-- (LLVM_libs_loc .. "/LLVMMipsDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMMipsInfo"),
		-- (LLVM_libs_loc .. "/LLVMMIRParser"),
		-- (LLVM_libs_loc .. "/LLVMMSP430AsmParser"),
		-- (LLVM_libs_loc .. "/LLVMMSP430CodeGen"),
		-- (LLVM_libs_loc .. "/LLVMMSP430Desc"),
		-- (LLVM_libs_loc .. "/LLVMMSP430Disassembler"),
		-- (LLVM_libs_loc .. "/LLVMMSP430Info"),
		-- (LLVM_libs_loc .. "/LLVMNVPTXCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMNVPTXDesc"),
		-- (LLVM_libs_loc .. "/LLVMNVPTXInfo"),
		(LLVM_libs_loc .. "/LLVMObjCARCOpts"),
		(LLVM_libs_loc .. "/LLVMObjCopy"),
		(LLVM_libs_loc .. "/LLVMObject"),
		(LLVM_libs_loc .. "/LLVMObjectYAML"),
		(LLVM_libs_loc .. "/LLVMOption"),
		(LLVM_libs_loc .. "/LLVMOrcJIT"),
		(LLVM_libs_loc .. "/LLVMOrcShared"),
		(LLVM_libs_loc .. "/LLVMOrcTargetProcess"),
		(LLVM_libs_loc .. "/LLVMPasses"),
		-- (LLVM_libs_loc .. "/LLVMPowerPCAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMPowerPCCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMPowerPCDesc"),
		-- (LLVM_libs_loc .. "/LLVMPowerPCDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMPowerPCInfo"),
		(LLVM_libs_loc .. "/LLVMProfileData"),
		(LLVM_libs_loc .. "/LLVMRemarks"),
		-- (LLVM_libs_loc .. "/LLVMRISCVAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMRISCVCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMRISCVDesc"),
		-- (LLVM_libs_loc .. "/LLVMRISCVDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMRISCVInfo"),
		-- (LLVM_libs_loc .. "/LLVMRISCVTargetMCA"),
		(LLVM_libs_loc .. "/LLVMRuntimeDyld"),
		(LLVM_libs_loc .. "/LLVMScalarOpts"),
		(LLVM_libs_loc .. "/LLVMSelectionDAG"),
		-- (LLVM_libs_loc .. "/LLVMSparcAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMSparcCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMSparcDesc"),
		-- (LLVM_libs_loc .. "/LLVMSparcDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMSparcInfo"),
		(LLVM_libs_loc .. "/LLVMSupport"),
		(LLVM_libs_loc .. "/LLVMSymbolize"),
		-- (LLVM_libs_loc .. "/LLVMSystemZAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMSystemZCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMSystemZDesc"),
		-- (LLVM_libs_loc .. "/LLVMSystemZDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMSystemZInfo"),
		(LLVM_libs_loc .. "/LLVMTableGen"),
		(LLVM_libs_loc .. "/LLVMTableGenCommon"),
		(LLVM_libs_loc .. "/LLVMTableGenGlobalISel"),
		(LLVM_libs_loc .. "/LLVMTarget"),
		(LLVM_libs_loc .. "/LLVMTargetParser"),
		(LLVM_libs_loc .. "/LLVMTextAPI"),
		(LLVM_libs_loc .. "/LLVMTransformUtils"),
		-- (LLVM_libs_loc .. "/LLVMVEAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMVECodeGen"),
		(LLVM_libs_loc .. "/LLVMVectorize"),
		-- (LLVM_libs_loc .. "/LLVMVEDesc"),
		-- (LLVM_libs_loc .. "/LLVMVEDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMVEInfo"),
		-- (LLVM_libs_loc .. "/LLVMWebAssemblyAsmParser"),
		-- (LLVM_libs_loc .. "/LLVMWebAssemblyCodeGen"),
		-- (LLVM_libs_loc .. "/LLVMWebAssemblyDesc"),
		-- (LLVM_libs_loc .. "/LLVMWebAssemblyDisassembler"),
		-- (LLVM_libs_loc .. "/LLVMWebAssemblyInfo"),
		-- (LLVM_libs_loc .. "/LLVMWebAssemblyUtils"),
		(LLVM_libs_loc .. "/LLVMWindowsDriver"),
		(LLVM_libs_loc .. "/LLVMWindowsManifest"),
		(LLVM_libs_loc .. "/LLVMX86AsmParser"),
		(LLVM_libs_loc .. "/LLVMX86CodeGen"),
		(LLVM_libs_loc .. "/LLVMX86Desc"),
		(LLVM_libs_loc .. "/LLVMX86Disassembler"),
		(LLVM_libs_loc .. "/LLVMX86Info"),
		(LLVM_libs_loc .. "/LLVMX86TargetMCA"),
		(LLVM_libs_loc .. "/LLVMXCoreCodeGen"),
		(LLVM_libs_loc .. "/LLVMXCoreDesc"),
		(LLVM_libs_loc .. "/LLVMXCoreDisassembler"),
		(LLVM_libs_loc .. "/LLVMXCoreInfo"),
		(LLVM_libs_loc .. "/LLVMXRay"),
		-- (LLVM_libs_loc .. "/LTO"),
		-- (LLVM_libs_loc .. "/Remarks"),

		-- (LLVM_libs_loc .. "/*.lib"),
	}
end






function link_clang()
	links {
		-- (LLVM_libs_loc .. "clangAnalysis"),
		-- (LLVM_libs_loc .. "clangAnalysisFlowSensitive"),
		-- (LLVM_libs_loc .. "clangAnalysisFlowSensitiveModels"),
		-- (LLVM_libs_loc .. "clangAPINotes"),
		-- (LLVM_libs_loc .. "clangARCMigrate"),
		-- (LLVM_libs_loc .. "clangAST"),
		-- (LLVM_libs_loc .. "clangASTMatchers"),
		-- (LLVM_libs_loc .. "clangBasic"),
		-- (LLVM_libs_loc .. "clangCodeGen"),
		-- (LLVM_libs_loc .. "clangCrossTU"),
		-- (LLVM_libs_loc .. "clangDependencyScanning"),
		-- (LLVM_libs_loc .. "clangDirectoryWatcher"),
		-- (LLVM_libs_loc .. "clangDriver"),
		-- (LLVM_libs_loc .. "clangDynamicASTMatchers"),
		-- (LLVM_libs_loc .. "clangEdit"),
		-- (LLVM_libs_loc .. "clangExtractAPI"),
		-- (LLVM_libs_loc .. "clangFormat"),
		-- (LLVM_libs_loc .. "clangFrontend"),
		-- (LLVM_libs_loc .. "clangFrontendTool"),
		-- (LLVM_libs_loc .. "clangHandleCXX"),
		-- (LLVM_libs_loc .. "clangHandleLLVM"),
		-- (LLVM_libs_loc .. "clangIndex"),
		-- (LLVM_libs_loc .. "clangIndexSerialization"),
		-- (LLVM_libs_loc .. "clangInstallAPI"),
		-- (LLVM_libs_loc .. "clangInterpreter"),
		-- (LLVM_libs_loc .. "clangLex"),
		-- (LLVM_libs_loc .. "clangParse"),
		-- (LLVM_libs_loc .. "clangRewrite"),
		-- (LLVM_libs_loc .. "clangRewriteFrontend"),
		-- (LLVM_libs_loc .. "clangSema"),
		-- (LLVM_libs_loc .. "clangSerialization"),
		-- (LLVM_libs_loc .. "clangStaticAnalyzerCheckers"),
		-- (LLVM_libs_loc .. "clangStaticAnalyzerCore"),
		-- (LLVM_libs_loc .. "clangStaticAnalyzerFrontend"),
		-- (LLVM_libs_loc .. "clangSupport"),
		-- (LLVM_libs_loc .. "clangTooling"),
		-- (LLVM_libs_loc .. "clangToolingASTDiff"),
		-- (LLVM_libs_loc .. "clangToolingCore"),
		-- (LLVM_libs_loc .. "clangToolingInclusions"),
		-- (LLVM_libs_loc .. "clangToolingInclusionsStdlib"),
		-- (LLVM_libs_loc .. "clangToolingRefactoring"),
		-- (LLVM_libs_loc .. "clangToolingSyntax"),
		-- (LLVM_libs_loc .. "clangTransformer"),
	}
end




function link_llvm_AArch64()
	links{
		(LLVM_libs_loc .. "LLVMAArch64AsmParser"),
		(LLVM_libs_loc .. "LLVMAArch64CodeGen"),
		(LLVM_libs_loc .. "LLVMAArch64Desc"),
		(LLVM_libs_loc .. "LLVMAArch64Disassembler"),
		(LLVM_libs_loc .. "LLVMAArch64Info"),
		(LLVM_libs_loc .. "LLVMAArch64Utils"),
	}
end


function link_llvm_AMDGPU()
	links{
		(LLVM_libs_loc .. "LLVMAMDGPUAsmParser"),
		(LLVM_libs_loc .. "LLVMAMDGPUCodeGen"),
		(LLVM_libs_loc .. "LLVMAMDGPUDesc"),
		(LLVM_libs_loc .. "LLVMAMDGPUDisassembler"),
		(LLVM_libs_loc .. "LLVMAMDGPUInfo"),
		(LLVM_libs_loc .. "LLVMAMDGPUTargetMCA"),
		(LLVM_libs_loc .. "LLVMAMDGPUUtils"),
	}
end

function link_llvm_ARM()
	links{
		(LLVM_libs_loc .. "LLVMARMAsmParser"),
		(LLVM_libs_loc .. "LLVMARMCodeGen"),
		(LLVM_libs_loc .. "LLVMARMDesc"),
		(LLVM_libs_loc .. "LLVMARMDisassembler"),
		(LLVM_libs_loc .. "LLVMARMInfo"),
		(LLVM_libs_loc .. "LLVMARMUtils"),
	}
end

function link_llvm_AVR()
	links{
		(LLVM_libs_loc .. "LLVMAVRAsmParser"),
		(LLVM_libs_loc .. "LLVMAVRCodeGen"),
		(LLVM_libs_loc .. "LLVMAVRDesc"),
		(LLVM_libs_loc .. "LLVMAVRDisassembler"),
		(LLVM_libs_loc .. "LLVMAVRInfo"),
	}
end

function link_llvm_BPF()
	links{
		(LLVM_libs_loc .. "LLVMBPFAsmParser"),
		(LLVM_libs_loc .. "LLVMBPFCodeGen"),
		(LLVM_libs_loc .. "LLVMBPFDesc"),
		(LLVM_libs_loc .. "LLVMBPFDisassembler"),
		(LLVM_libs_loc .. "LLVMBPFInfo"),
	}
end

function link_llvm_Exegesis()
	links{
		(LLVM_libs_loc .. "LLVMExegesis"),
		(LLVM_libs_loc .. "LLVMExegesisAArch64"),
		(LLVM_libs_loc .. "LLVMExegesisMips"),
		(LLVM_libs_loc .. "LLVMExegesisPowerPC"),
		(LLVM_libs_loc .. "LLVMExegesisX86"),
	}
end

function link_llvm_Hexagon()
	links{
		(LLVM_libs_loc .. "LLVMHexagonAsmParser"),
		(LLVM_libs_loc .. "LLVMHexagonCodeGen"),
		(LLVM_libs_loc .. "LLVMHexagonDesc"),
		(LLVM_libs_loc .. "LLVMHexagonDisassembler"),
		(LLVM_libs_loc .. "LLVMHexagonInfo"),
	}
end

function link_llvm_Lanai()
	links{
		(LLVM_libs_loc .. "LLVMLanaiAsmParser"),
		(LLVM_libs_loc .. "LLVMLanaiCodeGen"),
		(LLVM_libs_loc .. "LLVMLanaiDesc"),
		(LLVM_libs_loc .. "LLVMLanaiDisassembler"),
		(LLVM_libs_loc .. "LLVMLanaiInfo"),
	}
end

function link_llvm_LoongArch()
	links{
		(LLVM_libs_loc .. "LLVMLoongArchAsmParser"),
		(LLVM_libs_loc .. "LLVMLoongArchCodeGen"),
		(LLVM_libs_loc .. "LLVMLoongArchDesc"),
		(LLVM_libs_loc .. "LLVMLoongArchDisassembler"),
		(LLVM_libs_loc .. "LLVMLoongArchInfo"),
	}
end

function link_llvm_Mips()
	links{
		(LLVM_libs_loc .. "LLVMMipsAsmParser"),
		(LLVM_libs_loc .. "LLVMMipsCodeGen"),
		(LLVM_libs_loc .. "LLVMMipsDesc"),
		(LLVM_libs_loc .. "LLVMMipsDisassembler"),
		(LLVM_libs_loc .. "LLVMMipsInfo"),
	}
end

function link_llvm_MSP430()
	links{
		(LLVM_libs_loc .. "LLVMMSP430AsmParser"),
		(LLVM_libs_loc .. "LLVMMSP430CodeGen"),
		(LLVM_libs_loc .. "LLVMMSP430Desc"),
		(LLVM_libs_loc .. "LLVMMSP430Disassembler"),
		(LLVM_libs_loc .. "LLVMMSP430Info"),
	}
end

function link_llvm_NVPTX()
	links{
		(LLVM_libs_loc .. "LLVMNVPTXCodeGen"),
		(LLVM_libs_loc .. "LLVMNVPTXDesc"),
		(LLVM_libs_loc .. "LLVMNVPTXInfo"),
	}
end

function link_llvm_PowerPC()
	links{
		(LLVM_libs_loc .. "LLVMPowerPCAsmParser"),
		(LLVM_libs_loc .. "LLVMPowerPCCodeGen"),
		(LLVM_libs_loc .. "LLVMPowerPCDesc"),
		(LLVM_libs_loc .. "LLVMPowerPCDisassembler"),
		(LLVM_libs_loc .. "LLVMPowerPCInfo"),
	}
end

function link_llvm_RISCV()
	links{
		(LLVM_libs_loc .. "LLVMRISCVAsmParser"),
		(LLVM_libs_loc .. "LLVMRISCVCodeGen"),
		(LLVM_libs_loc .. "LLVMRISCVDesc"),
		(LLVM_libs_loc .. "LLVMRISCVDisassembler"),
		(LLVM_libs_loc .. "LLVMRISCVInfo"),
		(LLVM_libs_loc .. "LLVMRISCVTargetMCA"),
	}
end

function link_llvm_Sparc()
	links{
		(LLVM_libs_loc .. "LLVMSparcAsmParser"),
		(LLVM_libs_loc .. "LLVMSparcCodeGen"),
		(LLVM_libs_loc .. "LLVMSparcDesc"),
		(LLVM_libs_loc .. "LLVMSparcDisassembler"),
		(LLVM_libs_loc .. "LLVMSparcInfo"),
	}
end

function link_llvm_SystemZ()
	links{
		(LLVM_libs_loc .. "LLVMSystemZAsmParser"),
		(LLVM_libs_loc .. "LLVMSystemZCodeGen"),
		(LLVM_libs_loc .. "LLVMSystemZDesc"),
		(LLVM_libs_loc .. "LLVMSystemZDisassembler"),
		(LLVM_libs_loc .. "LLVMSystemZInfo"),
	}
end

function link_llvm_WASM()
	links{
		(LLVM_libs_loc .. "LLVMWebAssemblyAsmParser"),
		(LLVM_libs_loc .. "LLVMWebAssemblyCodeGen"),
		(LLVM_libs_loc .. "LLVMWebAssemblyDesc"),
		(LLVM_libs_loc .. "LLVMWebAssemblyDisassembler"),
		(LLVM_libs_loc .. "LLVMWebAssemblyInfo"),
		(LLVM_libs_loc .. "LLVMWebAssemblyUtils"),
	}
end

function link_llvm_X86()
	links{
		(LLVM_libs_loc .. "LLVMX86AsmParser"),
		(LLVM_libs_loc .. "LLVMX86CodeGen"),
		(LLVM_libs_loc .. "LLVMX86Desc"),
		(LLVM_libs_loc .. "LLVMX86Disassembler"),
		(LLVM_libs_loc .. "LLVMX86Info"),
		(LLVM_libs_loc .. "LLVMX86TargetMCA"),
	}
end

function link_llvm_XCore()
	links{
		(LLVM_libs_loc .. "LLVMXCoreCodeGen"),
		(LLVM_libs_loc .. "LLVMXCoreDesc"),
		(LLVM_libs_loc .. "LLVMXCoreDisassembler"),
		(LLVM_libs_loc .. "LLVMXCoreInfo"),
	}
end




function link_llvm()
	links{
		-- (LLVM_libs_loc .. "lldCOFF"),
		-- (LLVM_libs_loc .. "lldCommon"),
		-- (LLVM_libs_loc .. "lldELF"),
		-- (LLVM_libs_loc .. "lldMachO"),
		-- (LLVM_libs_loc .. "lldMinGW"),
		-- (LLVM_libs_loc .. "lldWasm"),
		-- platform: AArch64
		(LLVM_libs_loc .. "LLVMAggressiveInstCombine"),
		-- platform: AMDGPU
		(LLVM_libs_loc .. "LLVMAnalysis"),
		-- platform: ARM
		(LLVM_libs_loc .. "LLVMAsmParser"),
		(LLVM_libs_loc .. "LLVMAsmPrinter"),
		-- platform: AVR
		(LLVM_libs_loc .. "LLVMBinaryFormat"),
		(LLVM_libs_loc .. "LLVMBitReader"),
		(LLVM_libs_loc .. "LLVMBitstreamReader"),
		(LLVM_libs_loc .. "LLVMBitWriter"),
		-- platform: BPF
		(LLVM_libs_loc .. "LLVMCFGuard"),
		(LLVM_libs_loc .. "LLVMCFIVerify"),
		(LLVM_libs_loc .. "LLVMCodeGen"),
		(LLVM_libs_loc .. "LLVMCodeGenTypes"),
		(LLVM_libs_loc .. "LLVMCore"),
		(LLVM_libs_loc .. "LLVMCoroutines"),
		(LLVM_libs_loc .. "LLVMCoverage"),
		(LLVM_libs_loc .. "LLVMDebugInfoBTF"),
		(LLVM_libs_loc .. "LLVMDebugInfoCodeView"),
		(LLVM_libs_loc .. "LLVMDebuginfod"),
		(LLVM_libs_loc .. "LLVMDebugInfoDWARF"),
		(LLVM_libs_loc .. "LLVMDebugInfoGSYM"),
		(LLVM_libs_loc .. "LLVMDebugInfoLogicalView"),
		(LLVM_libs_loc .. "LLVMDebugInfoMSF"),
		(LLVM_libs_loc .. "LLVMDebugInfoPDB"),
		(LLVM_libs_loc .. "LLVMDemangle"),
		(LLVM_libs_loc .. "LLVMDiff"),
		(LLVM_libs_loc .. "LLVMDlltoolDriver"),
		(LLVM_libs_loc .. "LLVMDWARFLinker"),
		(LLVM_libs_loc .. "LLVMDWARFLinkerClassic"),
		(LLVM_libs_loc .. "LLVMDWARFLinkerParallel"),
		(LLVM_libs_loc .. "LLVMDWP"),
		(LLVM_libs_loc .. "LLVMExecutionEngine"),
		-- platform: Exegesis
		(LLVM_libs_loc .. "LLVMExtensions"),
		(LLVM_libs_loc .. "LLVMFileCheck"),
		-- (LLVM_libs_loc .. "LLVMFrontendDriver"),
		-- (LLVM_libs_loc .. "LLVMFrontendHLSL"),
		-- (LLVM_libs_loc .. "LLVMFrontendOffloading"),
		-- (LLVM_libs_loc .. "LLVMFrontendOpenACC"),
		-- (LLVM_libs_loc .. "LLVMFrontendOpenMP"),
		(LLVM_libs_loc .. "LLVMFuzzerCLI"),
		(LLVM_libs_loc .. "LLVMFuzzMutate"),
		(LLVM_libs_loc .. "LLVMGlobalISel"),
		-- platform: Hexagon
		(LLVM_libs_loc .. "LLVMHipStdPar"),
		(LLVM_libs_loc .. "LLVMInstCombine"),
		(LLVM_libs_loc .. "LLVMInstrumentation"),
		(LLVM_libs_loc .. "LLVMInterfaceStub"),
		(LLVM_libs_loc .. "LLVMInterpreter"),
		-- platform: (LLVM_libs_loc .. "LLVMipo"),
		(LLVM_libs_loc .. "LLVMIRPrinter"),
		(LLVM_libs_loc .. "LLVMIRReader"),
		(LLVM_libs_loc .. "LLVMJITLink"),
		-- platform: Lanai
		(LLVM_libs_loc .. "LLVMLibDriver"),
		(LLVM_libs_loc .. "LLVMLineEditor"),
		(LLVM_libs_loc .. "LLVMLinker"),
		-- platform: LoongArch
		(LLVM_libs_loc .. "LLVMLTO"),
		(LLVM_libs_loc .. "LLVMMC"),
		(LLVM_libs_loc .. "LLVMMCA"),
		(LLVM_libs_loc .. "LLVMMCDisassembler"),
		(LLVM_libs_loc .. "LLVMMCJIT"),
		(LLVM_libs_loc .. "LLVMMCParser"),
		-- platform: Mips
		(LLVM_libs_loc .. "LLVMMIRParser"),
		-- platform: MSP430
		-- platform: NVPTX
		(LLVM_libs_loc .. "LLVMObjCARCOpts"),
		(LLVM_libs_loc .. "LLVMObjCopy"),
		(LLVM_libs_loc .. "LLVMObject"),
		(LLVM_libs_loc .. "LLVMObjectYAML"),
		(LLVM_libs_loc .. "LLVMOptDriver"),
		(LLVM_libs_loc .. "LLVMOption"),
		(LLVM_libs_loc .. "LLVMOrcDebugging"),
		(LLVM_libs_loc .. "LLVMOrcJIT"),
		(LLVM_libs_loc .. "LLVMOrcShared"),
		(LLVM_libs_loc .. "LLVMOrcTargetProcess"),
		(LLVM_libs_loc .. "LLVMPasses"),
		-- platform: PowerPC
		(LLVM_libs_loc .. "LLVMProfileData"),
		(LLVM_libs_loc .. "LLVMRemarks"),
		-- platform: RISCV
		(LLVM_libs_loc .. "LLVMRuntimeDyld"),
		(LLVM_libs_loc .. "LLVMScalarOpts"),
		(LLVM_libs_loc .. "LLVMSelectionDAG"),
		-- platform: Sparc
		(LLVM_libs_loc .. "LLVMSupport"),
		(LLVM_libs_loc .. "LLVMSymbolize"),
		-- platform: SystemZ
		(LLVM_libs_loc .. "LLVMTableGen"),
		(LLVM_libs_loc .. "LLVMTableGenCommon"),
		(LLVM_libs_loc .. "LLVMTableGenGlobalISel"),
		(LLVM_libs_loc .. "LLVMTarget"),
		(LLVM_libs_loc .. "LLVMTargetParser"),
		(LLVM_libs_loc .. "LLVMTextAPI"),
		(LLVM_libs_loc .. "LLVMTextAPIBinaryReader"),
		(LLVM_libs_loc .. "LLVMTransformUtils"),
		(LLVM_libs_loc .. "LLVMVEAsmParser"),
		(LLVM_libs_loc .. "LLVMVECodeGen"),
		(LLVM_libs_loc .. "LLVMVectorize"),
		(LLVM_libs_loc .. "LLVMVEDesc"),
		(LLVM_libs_loc .. "LLVMVEDisassembler"),
		(LLVM_libs_loc .. "LLVMVEInfo"),
		-- platform: WASM
		(LLVM_libs_loc .. "LLVMWindowsDriver"),
		(LLVM_libs_loc .. "LLVMWindowsManifest"),
		-- platform: X86
		-- platform: XCore
		(LLVM_libs_loc .. "LLVMXRay"),
	}
end