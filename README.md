# Example usage of [Dyninst](https://github.com/dyninst/dyninst)

## Building

Before building any of these examples, you need an existing build of Dyninst (see the Dyninst [wiki](https://github.com/dyninst/dyninst/wiki/Building-Dyninst) for details).

To configure the build, you can use

    cmake . -DDyninst_DIR=path/to/Dyninst/install/lib/cmake/Dyninst

NOTE: The last three parts of the `Dyninst_DIR` path *must* be the `lib/cmake/Dyninst` directory under your Dyninst installation.

To build only specific examples, you can specify their names like so

    cmake --build <build_dir> --target codeCoverage

## Running

Each example is built in its own subdirectory under the main build directory (if you didn't specify `-B` when running CMake, the build directory is just the root of the `examples` project directory). For example, to run the codeCoverage example

	$ cd codeCoverage
	$ export DYNINSTAPI_RT_LIB=path/to/dyninst/lib/libdyninstAPI_RT.so
	$ export LD_LIBRARY_PATH=.:path/to/dyninst/lib:$LD_LIBRARY_PATH
	$ ./code_coverage -p testcc testcc.inst
	$ ./testcc.inst

## Issue with codeCoverage

When writing the instrumented library, it show a segmentation fault:

	$ ./code_coverage -s lib_to_instrument/libtorch_python.so libtorch_python.so
	Library instrumented, writing...
	Segmentation fault (core dumped)

	$ gdb --args  ./code_coverage -s lib_to_instrument/libtorch_python.so libtorch_python.so
	Thread 1 "code_coverage" received signal SIGSEGV, Segmentation fault.
	0x00007ffff7ec75df in Dyninst::Relocation::Instrumenter::handleCondDirExits(Dyninst::Relocation::RelocBlock*, Dyninst::Relocation::RelocGraph*, instPoint*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	(gdb) bt
	#0  0x00007ffff7ec75df in Dyninst::Relocation::Instrumenter::handleCondDirExits(Dyninst::Relocation::RelocBlock*, Dyninst::Relocation::RelocGraph*, instPoint*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#1  0x00007ffff7ec8425 in Dyninst::Relocation::Instrumenter::funcExitInstrumentation(Dyninst::Relocation::RelocBlock*, Dyninst::Relocation::RelocGraph*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#2  0x00007ffff7ec8603 in Dyninst::Relocation::Instrumenter::process(Dyninst::Relocation::RelocBlock*, Dyninst::Relocation::RelocGraph*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#3  0x00007ffff7ed9828 in Dyninst::Relocation::Transformer::processGraph(Dyninst::Relocation::RelocGraph*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#4  0x00007ffff7eb7596 in Dyninst::Relocation::CodeMover::transform(Dyninst::Relocation::Transformer&) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#5  0x00007ffff7d8afab in AddressSpace::transform(boost::shared_ptr<Dyninst::Relocation::CodeMover>) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#6  0x00007ffff7d8be8c in AddressSpace::relocateInt(std::_Rb_tree_const_iterator<func_instance*>, std::_Rb_tree_const_iterator<func_instance*>, unsigned long) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#7  0x00007ffff7d91e28 in AddressSpace::relocate() () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#8  0x00007ffff7ebe84a in Dyninst::PatchAPI::DynInstrumenter::run() () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#9  0x00007ffff75c9f75 in Dyninst::PatchAPI::Patcher::run() () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libpatchAPI.so.13.0
	#10 0x00007ffff75c97a4 in Dyninst::PatchAPI::Command::commit() () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libpatchAPI.so.13.0
	#11 0x00007ffff7d8cb50 in AddressSpace::patch(AddressSpace*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#12 0x00007ffff7e5e646 in BPatch_binaryEdit::writeFile(char const*) () from /storage/users/jalcaraz/spack/opt/spack/linux-cascadelake/dyninst-13.0.0-25z67skqvukyg4xbbjkzzfgcczfe3byr/lib/libdyninstAPI.so.13.0
	#13 0x0000555555559c24 in tauRewriteLibrary(BPatch*) ()
	#14 0x0000555555559f3e in main ()




How to reproduce

	$ cd codeCoverage
	$ ./code_coverage -s lib_to_instrument/libtorch_python.so libtorch_python.so

Seems to only throw the segmentation fault due to line 218 of code_coverage.C

	BPatchSnippetHandle* handle_b = mutateeAddressSpace->insertSnippet(exitTrace, *funcExit, BPatch_callAfter, BPatch_lastSnippet);

The error is not shown if that line is removed, but I would like to instrument both.
	



