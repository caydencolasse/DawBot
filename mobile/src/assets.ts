import type { ImageSourcePropType } from 'react-native';

export const images = {
  logo: require('../assets/images/logo-white.png') as ImageSourcePropType,
  robotFront: require('../assets/images/dawbot-front.jpg') as ImageSourcePropType,
  robotWave: require('../assets/images/dawbot-wave.jpg') as ImageSourcePropType,
};

export type DawId = 'ableton' | 'logic' | 'fl' | 'protools' | 'studiopro' | 'cubase' | 'garageband' | 'unsure';

export const DAWS: { id: DawId; name: string; icon: ImageSourcePropType | null }[] = [
  { id: 'ableton', name: 'Ableton Live', icon: require('../assets/images/daw/ableton.png') },
  { id: 'logic', name: 'Logic Pro', icon: require('../assets/images/daw/logic.png') },
  { id: 'fl', name: 'FL Studio', icon: require('../assets/images/daw/flstudio.png') },
  { id: 'protools', name: 'Pro Tools', icon: require('../assets/images/daw/protools.png') },
  { id: 'studiopro', name: 'Fender Studio Pro', icon: require('../assets/images/daw/studiopro.png') },
  { id: 'cubase', name: 'Cubase', icon: require('../assets/images/daw/cubase.png') },
  { id: 'garageband', name: 'GarageBand', icon: require('../assets/images/daw/garageband.png') },
  { id: 'unsure', name: 'Not sure yet', icon: null },
];

export const dawById = (id: DawId) => DAWS.find((d) => d.id === id) ?? DAWS[0];
