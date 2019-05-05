#include <stdio.h>
#include "trie_tree.h"

int read_ip_port(FILE *input, u32 *ip, u32 *mask)
{
	int i1, i2, i3, i4, port;
	int ret = fscanf(input, "%d.%d.%d.%d%u%d", &i1, &i2, &i3, &i4, mask, &port);
	if (ret != 6) {
		printf("parse ip-mask string error(ret = %d)!\n", ret);
		return 0;
	}

	*ip = (i1 << 24) | (i2 << 16) | (i3 << 8) | i4;

	return 1;
}

void dump_iptree(trie_node_t *tree, int tab)
{
    trie_node_t *p = tree;
    if(p->isIP)
    {
        for(int i = 0; i < tab; i++)
            printf(" ");
        //printf("%d "IP_FMT" %u\n", p->isIP, HOST_IP_FMT_STR(p->ip), p->mask);
    }
    for(int k = 0; k < 2; k++)
        if(p->child[k] != NULL)
            dump_iptree(p->child[k], tab+1);
}

int main()
{
    FILE *input = fopen("forwarding-table.txt", "r");
    iptree = new_trie_node(0);
    u32 ip, mask;
    while(read_ip_port(input, &ip, &mask))
    {
        int ret = insert_trie_node(ip, mask);
    }
    fclose(input);

    //dump_iptree(iptree, 0);

    input = fopen("forwarding-table.txt", "r");
    FILE *output = fopen("lookup-table.txt", "wr");
    while(read_ip_port(input, &ip, &mask))
    {
        u32 p = trie_lookup(ip, mask);
        fprintf(output, IP_FMT" %u\n", HOST_IP_FMT_STR(ip), p);
    }
    fclose(input);
    fclose(output);

    return 0;
}