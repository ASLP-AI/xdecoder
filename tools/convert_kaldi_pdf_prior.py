#!/usr/bin/python

# Created on 2018-06-16
# Author: Binbin Zhang

import sys
import math
import struct

def error_msg(msg):
    print('error: '+ msg)
 
def convert_pdf_prior(kaldi_pdf_prior_file, out_file):
    pdf_counts = None
    with open(kaldi_pdf_prior_file, 'r') as fid:
        # kaldi binary file start with '\0B'
        if fid.read(2) == '\0B':
            error_msg('''kaldi pdf prior binary file is not supported''')
        fid.seek(0)
        arr = fid.read().split()
        assert(arr[0] == '[')
        assert(arr[-1] == ']')
        pdf_counts = [float(e) for e in arr[1:-1]]
        print('total pdf count %d' % len(pdf_counts))
        total = sum(pdf_counts)
        for i in range(len(pdf_counts)):
            pdf_counts[i] /= total
            pdf_counts[i] += 1e-20
            pdf_counts[i] = math.log(pdf_counts[i])
            # print(pdf_counts[i])

    with open(out_file, 'wb') as fid:
        fid.write(struct.pack('<i', len(pdf_counts)))
        for e in pdf_counts:
            fid.write(struct.pack('<f', e))

if __name__ == '__main__':
    usage = '''Usage: convert kaldi pdf prior to net matrix 
               eg: convert_kaldi_pdf_prior.py pdf_prior_file out_net_file'''
    if len(sys.argv) != 3:
        error_msg(usage)
    convert_pdf_prior(sys.argv[1], sys.argv[2])

