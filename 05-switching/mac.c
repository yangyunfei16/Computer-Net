#include "mac.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

mac_port_map_t mac_port_map;

// initialize mac_port table
void init_mac_port_table()
{
	bzero(&mac_port_map, sizeof(mac_port_map_t));

	for (int i = 0; i < HASH_8BITS; i++) {
		init_list_head(&mac_port_map.hash_table[i]);
	}

	pthread_mutex_init(&mac_port_map.lock, NULL);

	pthread_create(&mac_port_map.thread, NULL, sweeping_mac_port_thread, NULL);
}

// destroy mac_port table
void destory_mac_port_table()
{
	pthread_mutex_lock(&mac_port_map.lock);
	mac_port_entry_t *entry, *q;
	for (int i = 0; i < HASH_8BITS; i++) {
		list_for_each_entry_safe(entry, q, &mac_port_map.hash_table[i], list) {
			list_delete_entry(&entry->list);
			free(entry);
		}
	}
	pthread_mutex_unlock(&mac_port_map.lock);
}

// compare mac address
int cmp_mac_addr(u8 *mac1, u8 *mac2)
{
	for (int i = 0; i < ETH_ALEN; i++)
	{
		if (mac1[i] != mac2[i])
			return 0;
	}
	return 1;
}

// lookup the mac address in mac_port table
iface_info_t *lookup_port(u8 mac[ETH_ALEN])
{
	// TODO: implement the lookup process here
	u8 ind = hash8((char *)mac, ETH_ALEN);

	mac_port_entry_t *entry = NULL;
	pthread_mutex_lock(&mac_port_map.lock);
	list_for_each_entry(entry, &mac_port_map.hash_table[ind], list)
	{
		if (cmp_mac_addr(mac, entry->mac))
		{
			entry->visited = time(NULL);
			pthread_mutex_unlock(&mac_port_map.lock);
			return entry->iface;
		}
	}

	pthread_mutex_unlock(&mac_port_map.lock);
	return NULL;
}

// insert the mac -> iface mapping into mac_port table
void insert_mac_port(u8 mac[ETH_ALEN], iface_info_t *iface)
{
	// TODO: implement the insertion process here
	u8 ind = hash8((char *)mac, ETH_ALEN);
	
	mac_port_entry_t *entry = malloc(sizeof(mac_port_entry_t));
	bzero(entry, sizeof(mac_port_entry_t));

	init_list_head(&entry->list);
	for (int i = 0; i < ETH_ALEN; i++)
		entry->mac[i] = mac[i];
	entry->iface = iface;
	entry->visited = time(NULL);

	pthread_mutex_lock(&mac_port_map.lock);
	list_add_tail(&entry->list, &mac_port_map.hash_table[ind]);
	pthread_mutex_unlock(&mac_port_map.lock);
}

// dumping mac_port table
void dump_mac_port_table()
{
	mac_port_entry_t *entry = NULL;
	time_t now = time(NULL);

	fprintf(stdout, "dumping the mac_port table:\n");
	pthread_mutex_lock(&mac_port_map.lock);
	for (int i = 0; i < HASH_8BITS; i++) {
		list_for_each_entry(entry, &mac_port_map.hash_table[i], list) {
			fprintf(stdout, ETHER_STRING " -> %s, %d\n", ETHER_FMT(entry->mac), \
					entry->iface->name, (int)(now - entry->visited));
		}
	}

	pthread_mutex_unlock(&mac_port_map.lock);
}

// sweeping mac_port table, remove the entry which has not been visited in the
// last 30 seconds.
int sweep_aged_mac_port_entry()
{
	// TODO: implement the sweeping process here
	int res = 0;
	mac_port_entry_t *entry = NULL;
	time_t now = time(NULL);

	pthread_mutex_lock(&mac_port_map.lock);
	for (int i = 0; i < HASH_8BITS; i++) {
		list_for_each_entry(entry, &mac_port_map.hash_table[i], list) {
			if (now - entry->visited >= 30)
			{
				list_delete_entry(&entry->list);
				res ++;
			}
		}
	}

	pthread_mutex_unlock(&mac_port_map.lock);

	return res;
}

// sweeping mac_port table periodically, by calling sweep_aged_mac_port_entry
void *sweeping_mac_port_thread(void *nil)
{
	while (1) {
		sleep(1);
		int n = sweep_aged_mac_port_entry();

		if (n > 0)
			log(DEBUG, "%d aged entries in mac_port table are removed.", n);
	}

	return NULL;
}
