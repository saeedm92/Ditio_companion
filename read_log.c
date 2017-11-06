#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
//#include "include/aes.h"
#define printk1 printf

int st_c = 0;
struct start_stop_t
{
	int pn;
	uint64_t t_start;
	uint64_t t_stop;
};
struct start_stop_t st_t[1000];

struct reg_wr
{
	uint32_t addr;
	uint32_t val;
	uint64_t t;
};
struct reg_wr regw[1000];

struct reg_r
{
	uint32_t addr;
	uint64_t t;
};
struct reg_r regr[1000];

struct phone_on_off
{
	uint64_t t_on;
	uint64_t t_off;
};
struct phone_on_off on_off[1000];

int add_start_t(int pn, uint64_t t)
{
	st_t[st_c].pn = pn;
	st_t[st_c].t_start = t;
	st_c++;
	printk1("start time added!\n");
	return 1;
}

int add_stop_t(int pn, uint64_t t)
{
	int i;
	for( i=0; i<st_c; i++ ) {
		if( st_t[i].pn == pn ) {
			st_t[i].t_stop = t;
			printk1("stop time added!\n");
			return 1;
		}
	}
	return 0;
}

int regw_c = 0, regr_c = 0;
void add_regw(uint32_t addr, uint32_t val, uint64_t t)
{
	regw[regw_c].addr = addr;
	regw[regw_c].val = val;
	regw[regw_c].t = t;
	regw_c++;
	//printk1("Reg: 0x%x\n", addr);
	//printk1("Val: %#x\n", val);
	//printk1("t: %lu\n", t);
	//	printk1("regw added!\n");
}

void add_regr(uint32_t addr, uint64_t t)
{
	regr[regr_c].addr = addr;
	regr[regr_c].t = t;
	regr_c++;
}

int on_off_c = 0;
void add_phone_off(int t)
{
	on_off[on_off_c].t_off = t;
}

void add_phone_on(int t)
{
	on_off[on_off_c].t_on = t;
}	
//Check if the device was logging at the specific address and period of time
bool q_check_time(uint32_t pn, uint64_t t1, uint64_t t2)
{
	int i;
	for( i=0; i<st_c; i++ ) {
		if( st_t[i].pn == pn ) {
			if( st_t[i].t_start<=t1 && st_t[i].t_stop>=t2  )
				return true;
		}
	}
	return false;
}

//Check if the device wasn't turned off during the logging
bool q_check_reboot(int pn)
{
	int i, j;
	for( i=0; i<st_c; i++ ) {
		bool tmp = false;
		for( j=0; j<on_off_c; j++ ) {
			if( (st_t[i].t_start > on_off[j].t_on) && st_t[i].t_stop < on_off[j].t_off) {
				tmp = true;
				break;
			}
		}
		if(!tmp)
			return false;
	}
	return true;
}

int main()
{
	//mbedtls_aes_context aes;
	FILE *fp;
	int ret, i;
	char op[1], pn[8], t[8], n[4], addr[8], val[1], regacess[3], master_iomem[256], slave_iomem[256], dump[4096];

        int master_iomem_p, slave_iomem_p;

	int file_index = 0;
	char file_name[10];
	while(1) {
		//opening new log file
		sprintf(file_name, "log%d", file_index);
		fp = fopen(file_name, "r");
		if (!fp) {
			break;
		}
		else {
			printf("file opened:%s\n", file_name);
		}
		file_index++;
			
		while(1) {
			//Readling the log file	
			ret = fread(op, sizeof(uint8_t), 1, fp);
			if( ret!= 1 ) {
				// Moveing to the next log file. Let's set it to -1 re-find the least t
				break;
			}	
			if( (int)op[0]== 1) {
				printk1("start logging\n");
				ret = fread(pn, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
				ret = fread(t, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
				ret = fread(t, sizeof(uint8_t), 1, fp);
				if (ret != 1)
					break;
				ret = fread(dump, sizeof(uint8_t), 4096, fp);
				if (ret != 4096)
					break;
				add_start_t( *((int *) pn), *((int *) t));
			}
			else if( (int)op[0]== 2) { 
				printk1("stop logging\n");
				ret = fread(pn, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
				ret = fread(t, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
				add_stop_t( *((int *) pn), *((int *) t) ); 
			}
			else if( (int)op[0]== 3) { //phone on
				ret = fread(t, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
				add_phone_on(*((int *) t));
			}
			else if( (int)op[0]== 4) { //phone off
				ret = fread(t, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
				add_phone_off(*((int *) t));
			}
			else if( (int)op[0]== 5) { //counter
				ret = fread(n, sizeof(uint8_t), 4, fp);
				if (ret != 4)
					break;
				return 0;
			}
			else if( (int)op[0]== 6) { //time diff
				ret = fread(t, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
			}
			else if( (int)op[0]== 7) { //time change
				ret = fread(t, sizeof(uint8_t), 8, fp);
				if (ret != 8)
					break;
			}
			else if( (int)op[0]== 8) { // Register write
				printk1("reg write\n");
				uint32_t val32 = 0;
				int j;
				for( j=0; j<4; j++ ) {
					int val_t;				
					if(j!=0)
						ret = fread(op, sizeof(uint8_t), 1, fp);
					ret = fread(addr, sizeof(uint8_t), 8, fp);
					if (ret != 8)
						break;
					ret = fread(val, sizeof(uint8_t), 1, fp);
					if (ret != 1)
						break;

					ret = fread(t, sizeof(uint8_t), 8, fp);
					if (ret != 8)
						break;
					val_t = (*((uint8_t*)val))<<(8*j);
					val32 = val32 + val_t;
					printk1("val_t= %#x val32 = %#x ", val_t, val32);
					printk1("addr= %#x\n", *((int*)addr)&0xff);
				}
				printk1( "%lu\n",*((uint64_t*) t));
	       			add_regw(*((uint32_t *) addr) & 0xff, val32, *((uint64_t*) t));
		         }
	                else if( (int)op[0]== 9) {
				printk1("reg read\n");
				regr_c++;
	                        ret = fread(addr, sizeof(uint8_t), 8, fp);
	                        if (ret != 8)
	                                break;
	                        ret = fread(val, sizeof(uint8_t), 1, fp);
	                        if (ret != 1)
	                                break;
	                        ret = fread(t, sizeof(uint8_t), 8, fp);
	                        if (ret != 8)
	                                break;
				printk1( "%lu\n", *( (uint64_t*) t));
				add_regr(*((uint32_t *) addr) & 0xff, *((uint64_t *) t));
	                }
		
		}
	}

	// Passing to check

        master_iomem_p = master_iomem;
	slave_iomem_p = slave_iomem;
	char regaccess[4];	

	//mbedtls_aes_setkey_dec(&aes, aes_key, 128);
	//mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 48, iv,
	//		buf, output);

        for( i=0; i<regw_c; i++ ) {
        	regaccess[0] = (char) 6; //camera dev_id
                regaccess[1] = (char) regw[i].addr; //register access offset
                regaccess[2] = (char) regw[i].val; //register access value
                regaccess[3] = (char) regw[i].t; //register access time
                ret = ditio_main((int) master_iomem_p, (int) slave_iomem_p, (int) regaccess);
                if( ret!=0 ) {
                        printk1("Violation\n");
			goto exit;
                }
		//else
	        //        master_iomem[ regw[i].addr ] = regw[i].val;
        }	
	exit:
	return 0;
}

