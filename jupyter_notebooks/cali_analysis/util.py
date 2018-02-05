import json
from pygments import highlight, lexers, formatters

import pandas as pd


def pretty_json_dumps(json_object):
    formatted_json = json.dumps(json_object, sort_keys=False, indent=4)
    colorful_json = highlight(formatted_json, lexers.JsonLexer(), formatters.TerminalFormatter())
    return colorful_json


def samples_to_dataframe(samples, use_spark=False):
    if use_spark:
        raise NotImplementedError
    else:
        if isinstance(samples, pd.DataFrame):
            return samples
        else:
            return pd.DataFrame(samples)
