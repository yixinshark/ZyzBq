bin_PROGRAMS = smp8756_pca9535
smp8756_pca9535_SOURCES = smp8756_pca9535.c rmmmimplementation.c dbgimplementation.c


#lib_LTLIBRARIES = libsmp8756_pca9535.la
#libsmp8756_pca9535_la_SOURCES = smp8756_pca9535.c
AM_CFLAGS=-DRMBUILD_USE_HWLIB_V2 \
		  -DRMCHIP_ID=RMCHIP_ID_SMP8756 \
		  -DRMCHIP_REVISION=1 \
		  -DEM86XX_MODE=EM86XX_MODEID_STANDALONE \
		  -DWITH_XLOADED_UCODE=1 \
		  -DWITHOUT_NERO_SPU=1 \
		  -DRMBUILD_USE_DCC_V2 \
		  -DWITH_OLD_CHIP_DEFINES=1 \
		  -DLLAD_DIRECT \
		  -D_DEBUG
smp8756_pca9535_CPPFLAGS=-I$(prefix)/include/mrua/display \
						   -I${prefix}/include/mrua/lllsi \
						   -I$(prefix)/include/mrua/base  \
						   -I${prefix}/include/mrua/playback \
						   -I${prefix}/include/mrua/hwdep_hwlib \
						   -I${prefix}/include/mrua/osal 


smp8756_pca9535_LDFLAGS = \
	-l8756_i2clib\
	-l8756_gpiolib	\
	-ldl \
	-lrua  \
						  -lllad  \
						  -lrmcore \
						  -lrmcw   \
						  -ldcc_v2 \
						  -lruaoutput \
						  -lrmoutputdebugger \
						  -lrmroute \
						  -lrmvideoout \
						  -lrmhdmi \
						  -lrmedid \
						  -lrminfoframe \
						  -lrmi2c \
						  -lrmsha1 \
						  -lrmcec \
						  -lrmhsi \
						  -lpthread \
						  -lruai2c \
						  -lm



#libsmp8756_pca9535_la_LDFLAGS = \
#	-l8756_i2clib\
#	-l8756_gpiolib 


