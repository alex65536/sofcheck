# How the engine works

SoFCheck is not significantly different from many other engines. It is based on recursive search
with alpha-beta pruning. Here, we describe the algorithms in some more details.

## Move generator

- board representation: simple 8x8 array + bitboards
- _PEXT Bitboards_ (if supported by hardware) and _Magic Bitboards_ (otherwise)

## Search

- _Alpha-Beta Search_ with _Principal Variation Search_
- _Iterative Deepening_
- _Quiescense Search_ for captures and pawn promotes to overcome horizon effect
- multithreading via _Lazy SMP_
- _Transposition Table_
- move ordering in the following order:
  - move from _Transposition Table_
  - captures ordered by _MVV-LVA_
  - pawn promotes
  - _Killer Heuristic_
  - _History Heuristic_
- _Futility Pruning_
- _Razoring_
- _Null Move Reduction_
- _Late Move Reduction_

## Evaluation

Evaluation function is linear to simplity its tuning. It includes:

- _Piece-Square Tables_
- _Tapered Eval_
- _King Safety_
  - _Pawn Shield_
  - _Pawn Storm_
  - queen or rook near opponent's king
- bonus for two bishops
- pawn structure: isolated, double, passed, protected and backward pawns
- open and semi-open lines

Coefficients for evaluation function are tuned using a method similar to _Texel's Tuning Method_,
but with slight modification. The method is described in this [post][tuning] (in Russian). You can
also refer to a [Jupyter notebook][tune-nb] used for tuning.

[tuning]: https://habr.com/ru/post/305604/
[tune-nb]: https://github.com/alex65536/sofcheck-weights-tuning/blob/master/notebooks/train.ipynb
