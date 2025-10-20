from math import *

def d_hamming(s1, s2):
    assert len(s1) == len(s2)
    return len([s1[i] for i in range(len(s1)) if s1[i] != s2[i]])

def hamming_encode(s):
    for c in s : assert c in {"0", "1"}

    ##Determiner la longueur finale du mot encodé
    parity_amnt = 0
    while 2**parity_amnt - parity_amnt < len(s) + 1: parity_amnt += 1  ##Formule mathématique pour déterminer la longueur finale en fonction des données ?
    res = ["_"] * (len(s)+parity_amnt)

    ##Copie des bits de données dans le résultat
    delta = 0
    for offset in range(parity_amnt):
        substr = s[delta:delta + 2**offset-1]
        for c in substr:
            res[1+delta + offset] = c
            delta += 1
    
    ##Calcul et insertion des bits de parité
    for parity in range(parity_amnt):
        par_sum = 0
        for offset in range(2**parity):
            checked_bits = res[2**parity-1+offset::2**(parity+1)]
            par_sum += sum([int(c) for c in checked_bits if c in {"0", "1"}])
        res[2**parity-1] = str(par_sum%2)
    return "".join(res)

def hamming_decode(s):
    for c in s : assert c in {"0", "1"}
    res = list(s)
    parity = 0
    while 2**parity-1 < len(s):
        res[2**parity-1] = ""
        parity += 1
    return "".join(res)   

def hamming_correct(s):
    for c in s : assert c in {"0", "1"}
    
    errors = []
    data_bits = ceil(log2(len(s)))
    for parity in range(len(s) - data_bits-1):
        par_sum = 0
        for offset in range(2**parity):
            checked_bits = s[2**parity-1+offset::2**(parity+1)]
            if offset == 0 : checked_bits = checked_bits[1:]
            par_sum += sum([int(c) for c in checked_bits])
        if str(par_sum%2) != s[2**parity-1] : errors.append(2**parity)

    if errors == [] : return s
    error_pos = sum(errors)-1
    return s[:error_pos] + str((int(s[error_pos])+1)%2) + s[error_pos+1:]

class Code:

    def __init__(self, alphabet, length, words=None):
        self._alphabet = alphabet
        self._length = length
        self._words = words if words != None else self.generate_words()
        assert len({len(w) for w in self._words}) == 1 ##Tous mots de même longueur
        assert len({c for w in self._words for c in w if c not in self._alphabet}) == 0 ##Tous caractère dans alphabet

    def generate_words(self):
        res = set()
        curr_word = [self._alphabet[0]] * self._length
        curr_number = [0] * self._length
        alphabet_len = len(self._alphabet)
        for i in range(alphabet_len ** self._length):
            change_index = 0
            while change_index != -1:
                if i%(alphabet_len**(change_index+1)) == 0 and i != 0:
                    curr_word[change_index] = self._alphabet[0]
                    curr_number[change_index] = 0
                    change_index += 1
                else :
                    curr_number[change_index] = (curr_number[change_index]+1)%len(self._alphabet)
                    curr_word[change_index] = self._alphabet[curr_number[change_index]] 
                    change_index = -1
            res.add("".join(curr_word))
        return res

    def __len__(self):
        return len(self._words)

    def __str__(self):
        return f"({self._alphabet}, {self._length}, {self._words})"

    def __iter__(self):
        return self._words.__iter__()

    def encode(self, encoding):
        first_item = True
        new_len = -1
        encoded_word = ""
        new_code = Code(self._alphabet, self._length, set())
        for word in self._words:
            encoded_word = encoding(word)
            if first_item : 
                new_len = len(encoded_word)
                new_code._length = new_len
                first_item = False
            if len(encoded_word) != new_len : raise TypeError("Length is inconsistent")
            new_code._words.add(encoded_word)
        if len(new_code) < len(self) : raise TypeError("Encoding is not injective")
        return new_code

    def d_min(self):
        return min({d_hamming(w1, w2) for w1 in self for w2 in self if w1 != w2})

    def summarize(self):
        return {"length" : self._length, "size" : len(self), "min_dist" : self.d_min()}

    def generator_matrix(self):
        return [[c for c in w] for w in self._words]

    def decode(self, recv_str):
        results = {w : d_hamming(recv_str, w) for w in self}
        min_dist = min(results.values())
        results = {w : results[w] for w in self if results[w] == min_dist}
        if len(results) > 1 : raise ValueError("Could not find unique match within code")
        return [w for w in results.keys()][0]

