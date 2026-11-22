import { Image, View } from 'react-native';
import { images } from '../../assets';
import { Brand, Button, Row, Screen, T } from '../../components/ui';

export default function Welcome() {
  return (
    <Screen
      footer={
        <>
          <Button label="Get started" href="/onboarding/daw" />
          <Button label="I already have an account" variant="ghost" href="/onboarding/account" />
        </>
      }
    >
      <View style={{ paddingHorizontal: 20, paddingTop: 16, flex: 1 }}>
        <Row gap={10}>
          <Image source={images.logo} style={{ width: 32, height: 32 }} accessibilityIgnoresInvertColors />
          <Brand />
        </Row>
        <View style={{ flex: 1, marginTop: 24, borderRadius: 24, overflow: 'hidden', backgroundColor: '#E2E2E2', maxHeight: 380 }}>
          <Image
            source={images.robotFront}
            resizeMode="cover"
            style={{ width: '100%', height: '100%' }}
            accessibilityLabel="The dawbot robot wearing headphones"
          />
        </View>
        <View style={{ gap: 12, paddingTop: 28 }}>
          <T v="display">Learn production on the go. Finish it in your studio</T>
          <T v="sub">Five-minute lessons, a home for every idea, and a plugin that picks up where you left off</T>
        </View>
      </View>
    </Screen>
  );
}
