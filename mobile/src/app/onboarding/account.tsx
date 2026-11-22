import { Flame, Laptop, Mail, Mic } from 'lucide-react-native';
import { View } from 'react-native';
import { Button, Card, Row, Screen, StepHeader, T, Title } from '../../components/ui';
import { colors } from '../../theme';

// TODO(auth): wire these to Sign in with Apple, Google and email magic links.
export default function CreateAccount() {
  return (
    <Screen
      scroll
      footer={
        <>
          <Button label="Continue with Apple" variant="light" href="/onboarding/link" />
          <Button label="Continue with Google" variant="secondary" href="/onboarding/link" />
          <Button label="Continue with email" variant="secondary" icon={Mail} href="/onboarding/link" />
          <T v="muted" style={{ textAlign: 'center', paddingTop: 4 }}>By continuing you agree to the Terms of Service and Privacy Policy</T>
        </>
      }
    >
      <StepHeader value={1} />
      <Title title="Save your progress" sub="Your streak, ideas and projects sync to the dawbot plugin on your computer" style={{ paddingHorizontal: 20, paddingTop: 8 }} />
      <Card style={{ margin: 20, marginTop: 32, padding: 20, gap: 16 }}>
        <Row>
          <Flame size={22} color={colors.coral} fill={colors.coral} />
          <T>Keep your streak on every device</T>
        </Row>
        <Row>
          <Mic size={22} color={colors.coralText} strokeWidth={1.75} />
          <T>Back up every idea you capture</T>
        </Row>
        <Row>
          <Laptop size={22} color={colors.coralText} strokeWidth={1.75} />
          <T>Open your projects in the plugin</T>
        </Row>
      </Card>
      <View />
    </Screen>
  );
}
