/*
 * Copyright (c) 2006-2007 Ed Schouten <ed@fxq.nl>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * @file vfs_dir.c
 */

#include "config.h"
#include "vfs_modules.h"

int
vfs_dir_open(struct vfsent *ve, int isdir)
{
	return (!isdir);
}

int
vfs_dir_populate(struct vfsent *ve)
{
	GDir *dir;
	const char *sfn;
	struct vfsref *new, *scur;
	int hide_dotfiles;

	hide_dotfiles = config_getopt_bool("vfs.dir.hide_dotfiles");

	if ((dir = g_dir_open(ve->filename, 0, NULL)) == NULL)
		return (-1);
	
	while ((sfn = g_dir_read_name(dir)) != NULL) {
		/* Hide dotted files */
		if (hide_dotfiles && sfn[0] == '.')
			continue;

		if ((new = vfs_open(sfn, NULL, ve->filename)) == NULL)
			continue;

		/*
		 * Add the items to the tailq in a sorted manner.
		 */
		vfs_list_foreach(&ve->population, scur) {
			/* Store the file if the sorting priority is lower */
			if (vfs_sortorder(new) < vfs_sortorder(scur) ||
			    /* Or if they are the same and the filename is lower */
			    (vfs_sortorder(new) == vfs_sortorder(scur) &&
			    strcasecmp(vfs_name(new), vfs_name(scur)) < 0)) {
				vfs_list_insert_before(&ve->population, new, scur);
				break;
			}
		}

		if (scur == NULL)
			vfs_list_insert_tail(&ve->population, new);
	}

	g_dir_close(dir);

	return (0);
}
