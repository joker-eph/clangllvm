//===--- Tools.h - Tool Implementations -------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_LIB_DRIVER_TOOLS_H_
#define CLANG_LIB_DRIVER_TOOLS_H_

#include "clang/Driver/Tool.h"
#include "clang/Driver/Types.h"
#include "clang/Driver/Util.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/Compiler.h"

namespace clang {
  class ObjCRuntime;

namespace driver {
  class Command;
  class Driver;

namespace toolchains {
  class MachO;
}

namespace tools {
using llvm::opt::ArgStringList;

  /// \brief Clang compiler tool.
  class LLVM_LIBRARY_VISIBILITY Clang : public Tool {
  public:
    static const char *getBaseInputName(const llvm::opt::ArgList &Args,
                                        const InputInfoList &Inputs);
    static const char *getBaseInputStem(const llvm::opt::ArgList &Args,
                                        const InputInfoList &Inputs);
    static const char *getDependencyFileName(const llvm::opt::ArgList &Args,
                                             const InputInfoList &Inputs);

  private:
    void AddPreprocessingOptions(Compilation &C, const JobAction &JA,
                                 const Driver &D,
                                 const llvm::opt::ArgList &Args,
                                 llvm::opt::ArgStringList &CmdArgs,
                                 const InputInfo &Output,
                                 const InputInfoList &Inputs) const;

    void AddAArch64TargetArgs(const llvm::opt::ArgList &Args,
                              llvm::opt::ArgStringList &CmdArgs) const;
    void AddARMTargetArgs(const llvm::opt::ArgList &Args,
                          llvm::opt::ArgStringList &CmdArgs,
                          bool KernelOrKext) const;
    void AddMIPSTargetArgs(const llvm::opt::ArgList &Args,
                           llvm::opt::ArgStringList &CmdArgs) const;
    void AddR600TargetArgs(const llvm::opt::ArgList &Args,
                           llvm::opt::ArgStringList &CmdArgs) const;
    void AddSparcTargetArgs(const llvm::opt::ArgList &Args,
                            llvm::opt::ArgStringList &CmdArgs) const;
    void AddSystemZTargetArgs(const llvm::opt::ArgList &Args,
                              llvm::opt::ArgStringList &CmdArgs) const;
    void AddX86TargetArgs(const llvm::opt::ArgList &Args,
                          llvm::opt::ArgStringList &CmdArgs) const;
    void AddHexagonTargetArgs(const llvm::opt::ArgList &Args,
                              llvm::opt::ArgStringList &CmdArgs) const;

    enum RewriteKind { RK_None, RK_Fragile, RK_NonFragile };

    ObjCRuntime AddObjCRuntimeArgs(const llvm::opt::ArgList &args,
                                   llvm::opt::ArgStringList &cmdArgs,
                                   RewriteKind rewrite) const;

    void AddClangCLArgs(const llvm::opt::ArgList &Args,
                        llvm::opt::ArgStringList &CmdArgs) const;

  public:
    Clang(const ToolChain &TC) : Tool("clang", "clang frontend", TC) {}

    bool hasGoodDiagnostics() const override { return true; }
    bool hasIntegratedAssembler() const override { return true; }
    bool hasIntegratedCPP() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  /// \brief Clang integrated assembler tool.
  class LLVM_LIBRARY_VISIBILITY ClangAs : public Tool {
  public:
    ClangAs(const ToolChain &TC) : Tool("clang::as",
                                        "clang integrated assembler", TC) {}

    bool hasGoodDiagnostics() const override { return true; }
    bool hasIntegratedAssembler() const override { return false; }
    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  /// gcc - Generic GCC tool implementations.
namespace gcc {
  class LLVM_LIBRARY_VISIBILITY Common : public Tool {
  public:
    Common(const char *Name, const char *ShortName,
           const ToolChain &TC) : Tool(Name, ShortName, TC) {}

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;

    /// RenderExtraToolArgs - Render any arguments necessary to force
    /// the particular tool mode.
    virtual void
        RenderExtraToolArgs(const JobAction &JA,
                            llvm::opt::ArgStringList &CmdArgs) const = 0;
  };

  class LLVM_LIBRARY_VISIBILITY Preprocess : public Common {
  public:
    Preprocess(const ToolChain &TC) : Common("gcc::Preprocess",
                                             "gcc preprocessor", TC) {}

    bool hasGoodDiagnostics() const override { return true; }
    bool hasIntegratedCPP() const override { return false; }

    void RenderExtraToolArgs(const JobAction &JA,
                             llvm::opt::ArgStringList &CmdArgs) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Compile : public Common  {
  public:
    Compile(const ToolChain &TC) : Common("gcc::Compile",
                                          "gcc frontend", TC) {}

    bool hasGoodDiagnostics() const override { return true; }
    bool hasIntegratedCPP() const override { return true; }

    void RenderExtraToolArgs(const JobAction &JA,
                             llvm::opt::ArgStringList &CmdArgs) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Link : public Common  {
  public:
    Link(const ToolChain &TC) : Common("gcc::Link",
                                       "linker (via gcc)", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void RenderExtraToolArgs(const JobAction &JA,
                             llvm::opt::ArgStringList &CmdArgs) const override;
  };
} // end namespace gcc

namespace hexagon {
  // For Hexagon, we do not need to instantiate tools for PreProcess, PreCompile and Compile.
  // We simply use "clang -cc1" for those actions.
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool {
  public:
    Assemble(const ToolChain &TC) : Tool("hexagon::Assemble",
      "hexagon-as", TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void RenderExtraToolArgs(const JobAction &JA,
                             llvm::opt::ArgStringList &CmdArgs) const;
    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Link : public Tool {
  public:
    Link(const ToolChain &TC) : Tool("hexagon::Link",
      "hexagon-ld", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    virtual void RenderExtraToolArgs(const JobAction &JA,
                                     llvm::opt::ArgStringList &CmdArgs) const;
    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace hexagon.

namespace arm {
  StringRef getARMTargetCPU(const llvm::opt::ArgList &Args,
                            const llvm::Triple &Triple);
  const char* getARMCPUForMArch(const llvm::opt::ArgList &Args,
                                const llvm::Triple &Triple);
  const char* getLLVMArchSuffixForARM(StringRef CPU);
}

namespace mips {
  bool hasMipsAbiArg(const llvm::opt::ArgList &Args, const char *Value);
}

namespace darwin {
  llvm::Triple::ArchType getArchTypeForMachOArchName(StringRef Str);
  void setTripleTypeForMachOArchName(llvm::Triple &T, StringRef Str);

  class LLVM_LIBRARY_VISIBILITY MachOTool : public Tool {
    virtual void anchor();
  protected:
    void AddMachOArch(const llvm::opt::ArgList &Args,
                       llvm::opt::ArgStringList &CmdArgs) const;

    const toolchains::MachO &getMachOToolChain() const {
      return reinterpret_cast<const toolchains::MachO&>(getToolChain());
    }

  public:
    MachOTool(const char *Name, const char *ShortName,
               const ToolChain &TC) : Tool(Name, ShortName, TC) {}
  };

  class LLVM_LIBRARY_VISIBILITY Assemble : public MachOTool  {
  public:
    Assemble(const ToolChain &TC) : MachOTool("darwin::Assemble",
                                              "assembler", TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Link : public MachOTool  {
    bool NeedsTempPath(const InputInfoList &Inputs) const;
    void AddLinkArgs(Compilation &C, const llvm::opt::ArgList &Args,
                     llvm::opt::ArgStringList &CmdArgs,
                     const InputInfoList &Inputs) const;

  public:
    Link(const ToolChain &TC) : MachOTool("darwin::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Lipo : public MachOTool  {
  public:
    Lipo(const ToolChain &TC) : MachOTool("darwin::Lipo", "lipo", TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Dsymutil : public MachOTool  {
  public:
    Dsymutil(const ToolChain &TC) : MachOTool("darwin::Dsymutil",
                                              "dsymutil", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isDsymutilJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY VerifyDebug : public MachOTool  {
  public:
    VerifyDebug(const ToolChain &TC) : MachOTool("darwin::VerifyDebug",
                                                 "dwarfdump", TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

}

  /// openbsd -- Directly call GNU Binutils assembler and linker
namespace openbsd {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("openbsd::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("openbsd::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace openbsd

  /// bitrig -- Directly call GNU Binutils assembler and linker
namespace bitrig {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("bitrig::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("bitrig::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace bitrig

  /// freebsd -- Directly call GNU Binutils assembler and linker
namespace freebsd {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("freebsd::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("freebsd::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace freebsd

  /// netbsd -- Directly call GNU Binutils assembler and linker
namespace netbsd {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {

  public:
    Assemble(const ToolChain &TC)
      : Tool("netbsd::Assemble", "assembler", TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {

  public:
    Link(const ToolChain &TC)
      : Tool("netbsd::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace netbsd

  /// Directly call GNU Binutils' assembler and linker.
namespace gnutools {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("GNU::Assemble", "assembler", TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("GNU::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
}
  /// minix -- Directly call GNU Binutils assembler and linker
namespace minix {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("minix::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("minix::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace minix

  /// solaris -- Directly call Solaris assembler and linker
namespace solaris {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("solaris::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("solaris::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace solaris

  /// auroraux -- Directly call GNU Binutils assembler and linker
namespace auroraux {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("auroraux::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("auroraux::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace auroraux

  /// dragonfly -- Directly call GNU Binutils assembler and linker
namespace dragonfly {
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool  {
  public:
    Assemble(const ToolChain &TC) : Tool("dragonfly::Assemble", "assembler",
                                         TC) {}

    bool hasIntegratedCPP() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
  class LLVM_LIBRARY_VISIBILITY Link : public Tool  {
  public:
    Link(const ToolChain &TC) : Tool("dragonfly::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output,
                      const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace dragonfly

  /// Visual studio tools.
namespace visualstudio {
  class LLVM_LIBRARY_VISIBILITY Link : public Tool {
  public:
    Link(const ToolChain &TC) : Tool("visualstudio::Link", "linker", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Compile : public Tool {
  public:
    Compile(const ToolChain &TC) : Tool("visualstudio::Compile", "compiler", TC) {}

    bool hasIntegratedAssembler() const override { return true; }
    bool hasIntegratedCPP() const override { return true; }
    bool isLinkJob() const override { return false; }

    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;

    Command *GetCommand(Compilation &C, const JobAction &JA,
                        const InputInfo &Output,
                        const InputInfoList &Inputs,
                        const llvm::opt::ArgList &TCArgs,
                        const char *LinkingOutput) const;
  };
} // end namespace visualstudio

namespace arm {
  StringRef getARMFloatABI(const Driver &D, const llvm::opt::ArgList &Args,
                         const llvm::Triple &Triple);
}
namespace XCore {
  // For XCore, we do not need to instantiate tools for PreProcess, PreCompile and Compile.
  // We simply use "clang -cc1" for those actions.
  class LLVM_LIBRARY_VISIBILITY Assemble : public Tool {
  public:
    Assemble(const ToolChain &TC) : Tool("XCore::Assemble",
      "XCore-as", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };

  class LLVM_LIBRARY_VISIBILITY Link : public Tool {
  public:
    Link(const ToolChain &TC) : Tool("XCore::Link",
      "XCore-ld", TC) {}

    bool hasIntegratedCPP() const override { return false; }
    bool isLinkJob() const override { return true; }
    void ConstructJob(Compilation &C, const JobAction &JA,
                      const InputInfo &Output, const InputInfoList &Inputs,
                      const llvm::opt::ArgList &TCArgs,
                      const char *LinkingOutput) const override;
  };
} // end namespace XCore.


} // end namespace toolchains
} // end namespace driver
} // end namespace clang

#endif // CLANG_LIB_DRIVER_TOOLS_H_
