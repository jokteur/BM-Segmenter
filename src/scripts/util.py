from operator import itemgetter
import re


def special_sort(ids):
    """Sort first by number, then alphabetically.

    Returns
    -------
    list : sorted list
    list : sorted argument list"""
    numbers = {}
    no_numbers = []

    for i, id in enumerate(ids):
        try:
            digit = [not x.isdigit() for x in id].index(True)
        except ValueError:
            digit = len(id)

        try:
            number = int(id[:digit])
            if not number in numbers:
                numbers[number] = []
            numbers[number].append((id, i))
        except:
            no_numbers.append((id, i))

    numbers = sorted(numbers.items())

    sorted_ids = []
    for _, values in numbers:
        sorted_ids += sorted(values)

    sorted_ids += sorted(no_numbers)

    return list(map(itemgetter(1), sorted_ids)), list(map(itemgetter(0), sorted_ids))


def make_safe_filename(s):
    def safe_char(c):
        if c.isalnum():
            return c
        else:
            return "_"

    stripped = "".join(safe_char(c) for c in s).rstrip("_")
    return re.sub("_{2,}", "_", stripped)
