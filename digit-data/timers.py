"""
Show some options for timer 0 (clock) and timer 1 (display strobe) configuration.
The PIC16LF15323 describes these in chapters 25-26.
"""


def timer0():
    f_sources = (
        31.00e3,      # LFINTOSC
       500.00e3,      # MFINTOSC
         8.00e6 / 4,  # FOSC/4
         8.00e6,      # FOSC
    )

    f_target = 1/60

    pre_bits = range(16)
    pre_factors = [2**b for b in pre_bits]
    post_factors = range(1, 17)
    t_max = 2**16
    epsilon = 1e-8

    print(f'{"f_source":>10}\t{"pre":>4}\t{"post":>5}\t{"timer"}\t{"err"}')

    for f_source in f_sources:
        for pre_factor in pre_factors:
            for post_factor in post_factors:
                f_adj = f_source / pre_factor / post_factor
                timer = round(f_adj / f_target)
                if timer > t_max:
                    continue
                f_act = f_adj / timer
                err = f_act / f_target - 1
                if abs(err) < epsilon:
                    print(f'{f_source:10}\t{pre_factor:4}\t{post_factor:5}\t{timer}\t{err}')
    print()


timer0()


def timer1():
    f_sources = (
        31.00e3,      # LFINTOSC
        32.00e3,      # MFINTOSC
       500.00e3,      # MFINTOSC
         8.00e6 / 4,  # FOSC/4
         8.00e6,      # FOSC
    )

    f_target = 500 * 14  # 14 pulses per strobe sequence

    pre_bits = range(4)
    pre_factors = [2**b for b in pre_bits]
    t_max = 2**16
    epsilon = 0.2

    print(f'{"f_source":>10}\t{"pre":>4}\t{"timer"}\t{"err":>8}')

    for f_source in f_sources:
        for pre_factor in pre_factors:
            f_adj = f_source / pre_factor
            timer = round(f_adj / f_target)
            if timer > t_max:
                continue
            f_act = f_adj / timer
            err = f_act / f_target - 1
            if abs(err) < epsilon:
                print(f'{f_source:10}\t{pre_factor:4}\t{timer:5}\t{err:8.1e}')


timer1()
