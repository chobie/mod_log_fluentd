mod_log_fluentd.la: mod_log_fluentd.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_log_fluentd.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_log_fluentd.la
