ifdef FD_HAS_HOSTED
ifdef FD_HAS_INT128
ifdef FD_HAS_ZSTD
$(call add-hdrs,fd_store.h fd_pending_slots.h)
$(call add-objs,fd_store fd_pending_slots,fd_disco)
$(call make-unit-test,test_pending_slots,test_pending_slots,fd_store fd_pending_slots fd_disco fd_util)
endif
endif
endif
