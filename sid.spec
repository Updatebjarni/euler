input {
  chip: array(3) of {
    voice: array(3) of {
      pitch: virtual_cv
      gate: logic
      saw: logic
      triangle: logic
      pulse: logic
      noise: logic
      pw: virtual_cv
      attack: virtual_cv
      decay: virtual_cv
      sustain: virtual_cv
      release: virtual_cv
      filter: logic
      sync: logic
      ringmod: logic
      test: logic
      }
    vol: virtual_cv
    hp: logic
    bp: logic
    lp: logic
    mute3: logic
    res: virtual_cv
    filtext: logic
    cutoff: virtual_cv
    output_select: number
    }
  }

output empty
