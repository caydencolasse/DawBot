import { Redirect } from 'expo-router';

// First launch goes through onboarding. Once accounts exist, check auth here and
// send signed-in users straight to /learn.
export default function Index() {
  return <Redirect href="/onboarding" />;
}
