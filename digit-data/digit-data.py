#!/usr/bin/env python3

digs = (
'''
  A  
F   B
     
E   C
  D  
''',

'''
     
    B
     
    C
     
''',

'''
  A  
    B
  G  
E    
  D  
''',

'''
  A  
    B
  G  
    C
  D  
''',

'''
     
F   B
  G  
    C
     
''',

'''
  A  
F    
  G  
    C
  D  
''',

'''
  A  
F    
  G  
E   C
  D  
''',

'''
  A  
    B
     
    C
     
''',

'''
  A  
F   B
  G  
E   C
  D  
''',

'''
  A  
F   B
  G  
    C
  D  
''',
)

with open('digits.h', 'w') as f:
    f.write(
        'static const uint8_t seg_patterns[10] = {\n'
        '    //GFEDCBA\n'
    )
    for i, dig in enumerate(digs):
        code = 0
        for s in range(7):
            letter = chr(ord('A') + s)
            if letter in dig:
                code |= 1 << s
        f.write(f'    0b{code:07b}, // {i}\n')
    f.write('};\n')
