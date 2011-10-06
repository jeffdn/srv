srv
October 2, 2011
Jeff Nettleton
jeffdn@gmail.com

This is an event-based, threaded HTTP server that is intended to address the C10k
problem. Has support for many of the standard webserver features, including:

 - directory listing, where allowed
 - protection from ../ path snooping
 - directories prefixed with . are hidden
 - ability to hide any folder (appears as 404)
 - /index.html in any folder
 - chroot user jails, with user and group switching (requires root)
 - basic support for modules that perform custom actions
 - ability to run on multiple ports at once
 - basic caching (based on time_t st_mtime)

Requirements:
 - libevent
 - libpthread
