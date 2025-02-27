import { getControlData } from "@/actions/actions";
import { Chart } from "./components/chart";
import ParameterController from "./components/parameter-controller";
import { ControlDataProvider } from "./hooks/useControlData";

const Page = () => {
  return (
    <div className="p-6 space-y-4">
      <ControlDataProvider parameterPromise={getControlData()}>
        <ParameterController />
        <Chart />
      </ControlDataProvider>
    </div>
  );
};

export default Page;
