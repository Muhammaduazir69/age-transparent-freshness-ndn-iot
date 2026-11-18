import csv,collections,sys
rows=list(csv.DictReader(open(sys.argv[1])))
M=sys.argv[2].split(',') if len(sys.argv)>2 else ['valid_cache_service','satisfaction','stale_delivery','latency_ms','router_valid_hit','router_stale_hit']
tab=collections.defaultdict(dict)
for r in rows: tab[(r['config'],r['regime'])][r['metric']]=(float(r['mean']),r['ci95'])
cfgs=sorted({c for c,_ in tab})
for reg in ['stationary','drift','pollution','flash','all']:
    print('==',reg)
    for c in cfgs:
        d=tab[(c,reg)]; g=lambda m:d.get(m,(float('nan'),''))[0]
        print(f"{c:18s}",' '.join(f"{m[:9]}={g(m):.4f}" for m in M), '|sh',' '.join(f"{g('share_E%d'%k):.2f}" for k in range(5)), '|gh',' '.join(f"{g('ghost_valid_E%d'%k):.3f}" for k in range(5)))
