import { getControlData } from "@/actions/actions";
import { Chart } from "./components/chart";
import { ControlDataProvider } from "./hooks/useControlData";

const Page = () => {
  return (
    <div className="p-6 space-y-4">
      <ControlDataProvider parameterPromise={getControlData()}>
        <Chart />
      </ControlDataProvider>
    </div>
  );
};

export default Page;
