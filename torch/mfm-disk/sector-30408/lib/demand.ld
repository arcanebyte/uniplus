-N
-F

MEMORY {
	valid : org = 0xa8, len =0x20000000
}

SECTIONS {
	.text : {}
	GROUP  ALIGN (0x400000) BLOCK(1024) : {
		.data :{}
		.bss BLOCK(1024):{}
	}


}
