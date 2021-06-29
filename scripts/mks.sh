#!/bin/sh
#Danmakufu shader maker
#Usage: (shaderc) (vs/fs/cs) (name) (in_dir) (out_dir) (bgfx_dir) (pe)

comp()
{
	#(shaderc) (rendertype) (vs/fs/cs) (name) (in) (out) (bgfx) (pe)
	
	# ms specific flags ...
	if [ "$3" = "vs" ] ; then 
		D3DX11="-p vs_5_0 -O 3"
		D3DX9="-p vs_3_0 -O 3"
	elif [ "$3" = "fs" ] ; then
		D3DX11="-p ps_5_0 -O 3"
		D3DX9="-p ps_3_0 -O 3"
	elif [ "$3" = "cs" ] ; then
		D3DX11="-p cs_5_0 -O 1"
		D3DX9=""
	fi
	
	if [ "$2" = 0 ] ; then
		ARGS="--platform windows $D3DX11"
		SDIR=hlsl5
	elif [ "$2" = 1 ] ; then
		ARGS="--platform windows $D3DX9"
		SDIR=hlsl3
	elif [ "$2" = 2 ] ; then
		ARGS="--platform linux -p 120"
		SDIR=glsl
	elif [ "$2" = 3 ] ; then
		ARGS="--platform android"
		SDIR=essl
	elif [ "$2" = 4 ] ; then
		ARGS="--platform osx -p metal"
		SDIR=metal
	elif [ "$2" = 5 ] ; then
		ARGS="--platform linux -p spirv"
		SDIR=spirv
	elif [ "$2" = 6 ] ; then
		ARGS="--platform orbis -p pssl2"
		SDIR=pssl
	elif [ "$2" = 7 ] ; then
		ARGS="--platform nacl"
		SDIR=nacl
	fi
	
	"$1" "$ARGS" --type "$3" -i "$7/src" -i "$7/examples/common" -f "$5/$8/$4.$3" -o "$6/$SDIR/$8/$4_$3.bin" --varyingdef "$5/$8/$4.def"
}

execcomp()
{
	#(shaderc) (vs/fs/cs) (in_name) (in_dir) (out_dir)
	comp "$1" 0 "$2" "$3" "$4" "$5" "$6" "$7"
	comp "$1" 1 "$2" "$3" "$4" "$5" "$6" "$7"
	comp "$1" 2 "$2" "$3" "$4" "$5" "$6" "$7"
	comp "$1" 3 "$2" "$3" "$4" "$5" "$6" "$7"
	comp "$1" 4 "$2" "$3" "$4" "$5" "$6" "$7"
	comp "$1" 5 "$2" "$3" "$4" "$5" "$6" "$7"
	#comp "$1" 6 "$2" "$3" "$4" "$5" "$6" "$7"
	#comp "$1" 7 "$2" "$3" "$4" "$5" "$6" "$7"
}

mkdir -p "$5/hlsl3/$7"
mkdir -p "$5/hlsl5/$7"
#mkdir -p "$5/pssl/$7"
#mkdir -p "$5/nacl/$7"
mkdir -p "$5/spirv/$7"
mkdir -p "$5/metal/$7"
mkdir -p "$5/glsl/$7"
mkdir -p "$5/essl/$7"

execcomp "$1" "$2" "$3" "$4" "$5" "$6" "$7"
