#ifndef __DEVICES_H__
#define __DEVICES_H__ 1
/*
 *
 * Device IDs are derived from 'https://www.kernel.org/doc/Documentation/admin-guide/devices.txt'
 * Implemented devices:
 * Block:
 * (3,0): "/dev/hda"
 * (3,64): "/dev/hdb"
 * (22,0): "/dev/hdc"
 * (22,64): "/dev/hdd"
 * Character:
 * (4, 1): "/dev/tty1"
 */
struct Dev {
	// TODO DO
}
Map* blockdevs;// Map, kdev_t -> struct Dev*
Map* chardevs;// Map, kdev_t -> struct Dev*
void initDevices(void) {
	bugCheck();
	// TODO IMPLEMENT
	//

}

#endif
