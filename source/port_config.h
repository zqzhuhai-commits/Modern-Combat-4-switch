/* SPDX-License-Identifier: MIT
 * port_config.h -- Modern Combat 4: Zero Hour Switch port configuration.
 */
#ifndef PORT_CONFIG_H
#define PORT_CONFIG_H

#define PORT_TITLE "现代战争4"
#define PORT_NAME "mc4_nx"
#define PORT_PACKAGE "com.gameloft.android.ANMP.GloftM4HM"
#define PORT_APK_DEFAULT_NAME "game.apk"
#define PORT_SO_REGION_BYTES (48u * 1024u * 1024u)

#define RT_SCREEN_W 1280
#define RT_SCREEN_H 720
#define RT_PATH_EXTRA_DIRS "logs", "data", "data/files", "data/cache"
#define RT_PATH_TRACE_EXTRA ".obb"
#define RT_OPENSLES 1
#define RT_GL_CHECK 1
#define RT_GL_SWAP_ENDS_FRAME 0
#define RT_BOOST_AT_BOOT 0
#endif
