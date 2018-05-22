import pandas

def CheckUnderinstrumentation(df, annotationkey='function', metric='count', samplekey='source.function#callpath.address', cutoff=20.0, min_percent=10.0):
    """ Check caliper data in the given Pandas dataframe for underinstrumentation. 
        
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

    anngrp = df.groupby(annotationkey)

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


if __name__ == "__main__":
    import json
    import sys

    with open(sys.argv[1]) as f:
        obj = json.load(f)

    annotationkey = 'function'
    if len(sys.argv) > 2:
        annotationkey = sys.argv[2]
        
    candidates = CheckUnderinstrumentation(pandas.DataFrame(obj), annotationkey)
    
    num_candidates = 0
    for count, cps in candidates.values():
        num_candidates += len(cps)

    print('There are {} candidates for additional annotations.'.format(num_candidates))
    
    for ann, cpinfo in candidates.iteritems():
        print('For region \"{}\": '.format(ann.rsplit('/', 1)[-1]))
        total, cps = cpinfo
        for cp, cpval in cps:
            print('  {}: {} ({:.1%})'.format(cp.rsplit('/', 1)[-1], cpval, float(cpval)/total))
