import itertools

DIMACS_INSTANCE_DIR = 'data/Color02'

GRAPHS = ['myciel3.col', 'myciel4.col', 'queen5_5.col', '1-FullIns_3.col', 'queen6_6.col', '2-Insertions_3.col',
          'myciel5.col', 'queen7_7.col', '2-FullIns_3.col', '3-Insertions_3.col', 'queen8_8.col', '1-Insertions_4.col',
          'huck.col', '4-Insertions_3.col', 'jean.col', '3-FullIns_3.col', 'queen9_9.col', 'david.col', 'mug88_25.col',
          'mug88_1.col', '1-FullIns_4.col', 'myciel6.col', 'queen8_12.col', 'mug100_1.col', 'queen10_10.col',
          'mug100_25.col', '4-FullIns_3.col', 'games120.col', 'queen11_11.col', 'DSJC125.9.col', 'DSJC125.1.col',
          'DSJC125.5.col', 'miles750.col', 'miles250.col', 'miles500.col', 'miles1500.col', 'miles1000.col', 'anna.col',
          'queen12_12.col', '2-Insertions_4.col', '5-FullIns_3.col', 'queen13_13.col', 'mulsol.i.3.col',
          'mulsol.i.4.col', 'mulsol.i.5.col', 'mulsol.i.2.col', 'myciel7.col', 'queen14_14.col', 'mulsol.i.1.col',
          '1-Insertions_5.col', 'zeroin.i.3.col', 'zeroin.i.2.col', 'zeroin.i.1.col', '2-FullIns_4.col',
          'queen15_15.col', 'DSJC250.1.col', 'DSJC250.5.col', 'DSJC250.9.col', 'queen16_16.col', '3-Insertions_4.col',
          '1-FullIns_5.col', 'school1_nsh.col', 'school1.col', '3-FullIns_4.col', 'fpsol2.i.3.col', 'le450_5c.col',
          'le450_15d.col', 'le450_15c.col', 'le450_15a.col', 'le450_25b.col', 'le450_25d.col', 'le450_5a.col',
          'le450_25a.col', 'le450_25c.col', 'le450_5d.col', 'le450_5b.col', 'le450_15b.col', 'fpsol2.i.2.col',
          '4-Insertions_4.col', 'fpsol2.i.1.col', 'DSJC500.5.col', 'DSJR500.1.col', 'DSJC500.1.col', 'DSJC500.9.col',
          'DSJR500.1c.col', 'DSJR500.5.col', 'homer.col', '2-Insertions_5.col', '1-Insertions_6.col', 'inithx.i.3.col',
          'inithx.i.2.col', 'ash331GPIA.col', '4-FullIns_4.col', 'will199GPIA.col', '2-FullIns_5.col', 'inithx.i.1.col',
          'latin_square_10.col', 'qg.order30.col', 'wap05a.col', 'wap06a.col', 'DSJC1000.5.col', 'DSJC1000.1.col',
          'DSJC1000.9.col', '5-FullIns_4.col', 'ash608GPIA.col', '3-Insertions_5.col', 'abb313GPIA.col',
          'qg.order40.col', 'wap07a.col', 'wap08a.col', 'ash958GPIA.col', '3-FullIns_5.col', 'wap01a.col', 'wap02a.col',
          'qg.order60.col', '4-FullIns_5.col', 'wap03a.col', 'wap04a.col', 'qg.order100.col']

MODELS = [
    (lambda model: model.add_color_orbitope_constraint(), 'Orbitope'),
    (lambda model: None, 'Default'),
    (lambda model: model.scip.setIntParam("misc/usesymmetry", 0), 'No-Sym'),
    (lambda model: model.add_color_orbitope_constraint(components_activation_handler=True, all_color_pairs=False), 'Act-Consec'),
    (lambda model: model.add_color_orbitope_constraint(components_activation_handler=True, all_color_pairs=True), 'Act-AllPairs'),
]

SELECTED_INSTANCES = [
    t
    for t in itertools.product(
        GRAPHS,
        [5, 6, 8, 10],
        MODELS,
    )
]
