in {
  chip: array(3) of {
    voice: array(3) of {
      pitch: int32
      gate: bool
      saw: bool
      triangle: bool
      pulse: bool
      noise: bool
      pw: int32
      attack: int32
      decay: int32
      sustain: int32
      release: int32
      filter: bool
      sync: bool
      ringmod: bool
      test: bool
      }
    vol: int32
    hp: bool
    bp: bool
    lp: bool
    mute3: bool
    res: int32
    filtext: bool
    cutoff: int32
    }
  mixer: int32
  }

out empty
