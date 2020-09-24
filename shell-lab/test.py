#!/usr/bin/env python
# python3 required!

import argparse
import os
import re
from collections import defaultdict

PID_REGEX = re.compile(r'\((\d+)\)')

def is_only_pid_different(output, ref):
  # '-e' not work in Mac's `/bin/echo`
  if '-e' in output:
    return True

  # SIGTSTP is 18 on MacOS, while 20 on Linux
  if 'stopped by signal 18' in output:
    output = output[:-2] + '20'
  
  output_search = PID_REGEX.search(output)
  ref_search = PID_REGEX.search(ref)
  if output_search and ref_search:
    return output.replace(output_search.group(0), '') == ref.replace(ref_search.group(0), '')
  return True

def test_ps_cmd(output, ref):
  output_ps_removed = []
  ref_ps_removed = []
  mysplit_stat_cnt = defaultdict(int)
  is_in_ps_context = False

  for l in ref:
    if not is_in_ps_context:
      ref_ps_removed.append(l)
    else:
      if './mysplit' in l:
        mysplit_stat_cnt[l.split()[2]] += 1
    
    if l.startswith('tsh> /bin/ps'):
      is_in_ps_context = True
    elif l.startswith('tsh>') or l.startswith('-e tsh>'):
      is_in_ps_context = False
  
  is_in_ps_context = False
  for l in output:
    if not is_in_ps_context:
      output_ps_removed.append(l)
    else:
      if './mysplit' in l:
        mysplit_stat_cnt[l.split()[2]] -= 1
    
    if l.startswith('tsh> /bin/ps'):
      is_in_ps_context = True
    elif l.startswith('tsh>') or l.startswith('-e tsh>'):
      is_in_ps_context = False

  return (all([v==0 for v in mysplit_stat_cnt.values()]), output_ps_removed, ref_ps_removed)

def test(i):
  stream = os.popen(f'./sdriver.pl -t trace{i:02}.txt -s ./tsh -a "-p"')
  output = [l.strip() for l in stream.read().splitlines()]

  with open('./tshref.out', 'r', encoding='utf-8') as f:
    while not f.readline().startswith(f'./sdriver.pl -t trace{i:02}.txt'):
      pass
    
    ref = []
    temp = f.readline()
    while temp and not temp.startswith('./sdriver.pl'):
      ref.append(temp.strip())
      temp = f.readline()

  ps_cmd_compare_passed, output, ref = test_ps_cmd(output, ref)

  if not ps_cmd_compare_passed:
    print('ps command compare failed')
    exit(0)

  if len(output) != len(ref):
    print('case: ' + str(i) + ' length not equal:\n>>>output\n\u001b[31m' + '\n'.join(output) + '\n\u001b[0m<<<ref\n\u001b[32m' + '\n'.join(ref) + '\u001b[0m')
    exit(0)
  
  for (outputline, refline) in zip(output, ref):
    if outputline != refline and not is_only_pid_different(outputline, refline):
      print(f'case{i:02} diff:\n\tout: \u001b[31m{outputline}\u001b[0m\n\tref: \u001b[32m{refline}\u001b[0m')
      exit(0)

  print(f'test case {i:02} passed')

def main():
  parser = argparse.ArgumentParser(description='test tsh lab')
  parser.add_argument('--case', default=0, type=int, help='0 for all case, (default: 0)', choices=range(17))
  args = parser.parse_args()

  if (args.case):
    test(args.case)
  else:
    # test all
    for i in range(1, 17):
      test(i)

if __name__ == "__main__":
    main()