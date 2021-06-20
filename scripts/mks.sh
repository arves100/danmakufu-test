#!/bin/sh
#Danmakufu shader maker
#Usage: (shaderc) (vs/fs/cs) (name) (in_dir) (out_dir) (bgfx_dir)

comp()
{
	#(shaderc) (n) (vs/fs/cs) (in) (out) (vd)
	
	# ms specific flags ...
	if $3=="vs" ; then 
		D3DX11=-p vs_5_0 -O 3
		D3DX9= -p vs_3_0 -O 3
	elif $3=="fs" ; then
		D3DX11=-p ps_5_0 -O 3
		D3DX9= -p ps_3_0 -O 3
	elif %3=="cs" ; then
		D3DX11=-p cs_5_0 -O 1
		D3DX9=
	fi
	
	if $2==0   ; then ARGS=--platform windows $D3DX11
	elif $2==1 ; then ARGS=--platform windows $D3DX9
	elif $2==2 ; then ARGS=--platform linux -p 120
	elif $2==3 ; then ARGS=--platform android
	elif $2==4 ; then ARGS=--platform osx -p metal
	elif $2==5 ; then ARGS=--platform linux -p spirv
	elif $2==6 ; then ARGS=--platform orbis -p pssl
	elif $2==7 ; then ARGS=--platform nacl
	fi
	"$~1" $ARGS --type $3 -i "$~6/src" -i "$~6/examples/common" -f "$~4" -o "$~5" --varyingdef "$~7"
}

execcomp()
{
	#(shaderc) (vs/fs/cs) (in_name) (in_dir) (out_dir)
	comp $1 0 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.hlsl5" $6 "$~4/$~3.def"
	comp $1 1 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.hlsl3" $6 "$~4/$~3.def"
	comp $1 2 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.glsl" $6 "$~4/$~3.def"
	comp $1 3 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.essl" $6 "$~4/$~3.def"
	comp $1 4 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.metal" $6 "$~4/$~3.def"
	comp $1 5 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.spirv" $6 "$~4/$~3.def"
	#comp $1 6 $2 "$~4/$~3.$~2" "$~5/$~3/$~2.pssl" "$~4/$~3.def"
	#comp $1 7 $2 "$~4/$~3.$2" "$~5/$~3/$~2.nacl" "$~4/$~3.def"
}

mkdir -p "$~5/$~3"
execcomp $1 $2 $3 $4 $5 $6
