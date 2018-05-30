"""
Caliper Annotation Guide

Input is a Caliper JSON file with event and call-path sampling data. 
One can be obtained with the following config:

CALI_SERVICES_ENABLE=callpath,event,report,sampler,symbollookup,timestamp,trace
CALI_SAMPLER_FREQUENCY=100
CALI_REPORT_CONFIG="select *,count(),sum(time.inclusive.duration) format json"
"""

import pandas

def CheckUnderinstrumentation(df, annotationkey='function', metric='count', samplekey='source.function#callpath.address', cutoff=20.0, min_percent=10.0):
    """ 
    Check Caliper data in the given Pandas dataframe for underinstrumentation. 
        
    Finds cases where there are multiple sampled locations for an annotated 
    region where a given metric exceeds a given percentage of the region's 
    total metric value.

    Returns a dictionary with the annotation candidates. The key is the 
    annotated region. The value is a tuple with the total metric value for
    the region, and a list of (sample callpath, metric value) tuples with
    the annotation candidates.

    annotation -> ( total value for metric, [ (sample callpath, sample callpath value), ... ] )
    """
    
    if not annotationkey in df.keys():
        raise RuntimeError('Annotation key \"{}\" is not in dataframe'.format(annotationkey))
    if not samplekey in df.keys():
        raise RuntimeError('Sample key \"{}\" is not in dataframe'.format(samplekey))
    if not metric in df.keys():
        raise RuntimeError('Metric \"{}\" is not in dataframe'.format(metric))

    dff = df[df['cali.event.begin'].isnull()] # Is there a less dumb way to do 
    dff = dff[dff['cali.event.end'].isnull()] # this?

    anngrp = dff.groupby(annotationkey)

    candidates = {}
    num_candidates = 0

    for ann, anndf in anngrp:
        totalval = anndf[metric].sum()

        if (totalval < cutoff):
            continue

        cpgrp = anndf.groupby(samplekey)
        cps   = []

        for cp, cpdf in cpgrp:
            cpval = cpdf[metric].sum()

            #   Add sampling loc to the preliminary candidate list if it
            # contributes more than min_percent of the annotation's total
            if (min_percent == 0.0 or cpval * (100.0/min_percent) > totalval):
                cps.append( (cp, cpval) )

        if len(cps) > 1:
            candidates[ann] = (totalval, cps)

    return candidates


def CheckOverinstrumentation(df, count_threshold_percent=2.0, time_threshold=100.0):
    """ 
    Check Caliper data in the given Pandas dataframe for overinstrumentation.

    Finds annotation regions which are called more than the given percentage
    of total annotations and whose time per invocation is less than the
    given time threshold.

    Returns a list of candidates. The elements are tuples of the form 
    (annotation, count, time) with the annotation region name, the number
    of times it was called, and the total time spent in the region.
    """
    
    keyset = [key for key in df.keys() if key.startswith('event.end#')]

    totalcount = df['count'].sum()
    totaltime  = df['time.inclusive.duration'].max()

    candidates = []

    for key in keyset:
        grp = df.groupby(key)

        for ann, anndf in grp:
            count = anndf['count'].sum()
            time  = anndf['time.inclusive.duration'].sum()

            if count*(100.0/count_threshold_percent) > totalcount and float(time)/count < time_threshold:
               candidates.append( (ann, count, time) )

    return candidates
    
    
if __name__ == "__main__":
    import json
    import sys

    with open(sys.argv[1]) as f:
        obj = json.load(f)

    annotationkey = 'function'
    metric        = 'count'
    
    if len(sys.argv) > 2:
        annotationkey = sys.argv[2]
    if len(sys.argv) > 3:
        metric        = sys.argv[3]

    df = pandas.DataFrame(obj)
    
    # check for underinstrumentation
    
    under = CheckUnderinstrumentation(df, annotationkey, metric)
    
    num_under = 0
    for count, cps in under.values():
        num_under += len(cps)

    print('There are {} candidates for additional annotations.'.format(num_under))
    
    for ann, cpinfo in under.iteritems():
        print('  For region \"{}\": '.format(ann.rsplit('/', 1)[-1]))
        total, cps = cpinfo
        for cp, cpval in cps:
            print('    {}: {} ({:.1%})'.format(cp.rsplit('/', 1)[-1], cpval, float(cpval)/total))

    # check for overinstrumentation

    over = CheckOverinstrumentation(df)

    print('\nThere are {} annotations that potentially cause high overhead.'.format(len(over)))

    totalcount = df['count'].sum()
    totaltime  = df['time.inclusive.duration'].max()

    for (annotation, count, time) in over:
        print('  {}: {} invocations ({:.1%}), ({:.1%} total time, {:.1f} usec per invocation)'.format(annotation, count, float(count)/totalcount, float(time)/totaltime, float(time)/count))
