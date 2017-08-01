LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
		source/aes.c					\
		source/arc4.c					\
		source/base64.c				\
		source/bignum.c				\
		source/camellia.c			\
		source/certs.c				\
		source/cipher.c				\
		source/cipher_wrap.c	\
		source/debug.c				\
		source/des.c					\
		source/dhm.c					\
		source/error.c				\
		source/havege.c				\
		source/md.c						\
		source/md_wrap.c			\
		source/md2.c					\
		source/md4.c					\
		source/md5.c					\
		source/net.c					\
		source/padlock.c			\
		source/pem.c					\
		source/pkcs11.c				\
		source/rsa.c					\
		source/sha1.c					\
		source/sha2.c					\
		source/sha4.c					\
		source/ssl_cli.c			\
		source/ssl_srv.c			\
		source/ssl_tls.c			\
		source/timing.c				\
		source/version.c			\
		source/x509parse.c		\
		source/xtea.c

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_EXPORT_LDLIBS := -lz

LOCAL_MODULE := libpolarssl

include $(BUILD_STATIC_LIBRARY)
