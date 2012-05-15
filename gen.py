#!/usr/bin/python

import string
import random

sccp = open('sccp.conf', 'w')
sccpsimple = open('sccp.conf.simple', 'w')

def randomMAC():
    mac = [ 0x00, 0x16, 0x3e,
        random.randint(0x00, 0x7f),
        random.randint(0x00, 0xff),
        random.randint(0x00, 0xff) ]
    return ''.join(map(lambda x: "%02X" % x, mac))

line_instance = 100
count = 0

sccp.write('[lines]\n')
while (count < 500):
 sccp.write('[' + str(line_instance) + ']\n')
 sccp.write('cid_num=' + str(line_instance) + '\n')
 sccp.write('cid_name=' + ''.join(random.choice(string.ascii_lowercase) for _ in xrange(5)) + '\n')
 sccp.write('\n')
 line_instance += 1
 count += 1 


line_instance = 100
count = 0

sccp.write('[devices]\n')
while (count < 500):
 mac = str(randomMAC())
 sccp.write('[SEP' + mac + ']\n')
 sccp.write('devices=' + str(mac) + '\n')
 sccp.write('line=' + str(line_instance) + '\n')
 sccp.write('\n')

 sccpsimple.write('SEP' + mac + ',' + str(line_instance) + '\n')

 line_instance += 1
 count += 1
