import os
from math import log
from collections import Counter
import pandas as pd
import numpy as np

def digit_ratio(query: str) -> float:
    return sum(c.isdigit() for c in query) / len(query)

def shannon_entropy(query: str) -> float:
    counts = Counter(query)
    frequencies = ((i / len(query)) for i in counts.values())
    return - sum(f * log(f, 2) for f in frequencies)

def length(query: str) -> int:
	return len(query)

def uppercase_ratio(query: str) -> float:
    return sum(c.isupper() for c in query) / len(query)

def extract_features(query: str):
	features = [
		float(length(query)),
		float(shannon_entropy(query)),
		float(digit_ratio(query)),
		float(uppercase_ratio(query))
	]
	return np.array(features)
