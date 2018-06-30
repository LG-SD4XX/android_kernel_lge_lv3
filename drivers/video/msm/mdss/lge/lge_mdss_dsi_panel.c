#include <linux/of_platform.h>
#include "mdss_dsi.h"

extern int mdss_dsi_parse_dcs_cmds(struct device_node *np, struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key);
extern void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl, struct dsi_panel_cmds *pcmds, u32 flags);

static int parse_dt_extra_dcs_cmds(struct device_node *np,
                        struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc;
	int i;
	const char *name;
	char buf1[256];
	char buf2[256];

	rc = of_property_count_strings(np, "lge,mdss-dsi-extra-command-names");
	if (rc > 0) {
		ctrl_pdata->lge_extra.num_extra_cmds = rc;
		pr_info("%s: num_extra_cmds=%d\n", __func__, ctrl_pdata->lge_extra.num_extra_cmds);
		ctrl_pdata->lge_extra.extra_cmds_array = kmalloc(sizeof(struct lge_cmds_entry)*ctrl_pdata->lge_extra.num_extra_cmds, GFP_KERNEL);
		if (NULL == ctrl_pdata->lge_extra.extra_cmds_array) {
			pr_err("%s: no memory\n", __func__);
			ctrl_pdata->lge_extra.num_extra_cmds = 0;
			return -ENOMEM;
		}
		for (i = 0; i < ctrl_pdata->lge_extra.num_extra_cmds; ++i) {
			of_property_read_string_index(np, "lge,mdss-dsi-extra-command-names", i, &name);
			strlcpy(ctrl_pdata->lge_extra.extra_cmds_array[i].name, name, sizeof(ctrl_pdata->lge_extra.extra_cmds_array[i].name));
			snprintf(buf1, sizeof(buf1), "lge,mdss-dsi-extra-command-%s", name);
			snprintf(buf2, sizeof(buf2), "lge,mdss-dsi-extra-command-state-%s", name);
			mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->lge_extra.extra_cmds_array[i].cmds, buf1, buf2);
		}
	} else {
		ctrl_pdata->lge_extra.num_extra_cmds = 0;
	}

	return 0;
}

int lge_mdss_panel_parse_dt_extra(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	int rc;
	u32 tmp;

	rc = of_property_read_u32(np, "lge,pre-on-cmds-delay", &tmp);
	ctrl_pdata->lge_extra.pre_on_cmds_delay = (!rc ? tmp : 0);

	rc = of_property_read_u32(np, "lge,post-ldo-on-delay", &tmp);
	ctrl_pdata->lge_extra.post_ldo_on_delay = (!rc ? tmp : 0);

	rc = of_property_read_u32(np, "lge,pre-bl-on-delay", &tmp);
	ctrl_pdata->lge_extra.pre_bl_on_delay = (!rc ? tmp : 0);

	parse_dt_extra_dcs_cmds(np, ctrl_pdata);

	return 0;
}

void lge_mdss_dsi_panel_extra_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl, const char *name)
{
	int i, index = -1;
	for (i = 0; i < ctrl->lge_extra.num_extra_cmds; ++i) {
		if (!strcmp(ctrl->lge_extra.extra_cmds_array[i].name, name)) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		if (ctrl->lge_extra.extra_cmds_array[index].cmds.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->lge_extra.extra_cmds_array[index].cmds, CMD_REQ_COMMIT);
	} else {
		pr_err("%s: extra cmds %s not found\n", __func__, name);
	}
}
