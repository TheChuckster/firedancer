$(call add-hdrs,fd_bmtree.h fd_wbmtree.h)
$(call add-objs,fd_wbmtree,fd_ballet)
$(call make-unit-test,test_bmtree,test_bmtree,fd_ballet fd_util)
$(call run-unit-test,test_bmtree)
