"use client";
import { updateControlData } from "@/actions/actions";
import { use, useCallback, useState } from "react";
import { ControlData, useControlData } from "../hooks/useControlData";

const defaultControlData: ControlData = {
  ki: 0.1,
  kd: 0.1,
  kp: 0.1,
  target: 90.0,
};

const InputControlParam = ({
  label,
  value,
  onChange,
}: {
  label: string;
  value: number;
  onChange?: (value: number) => void;
}) => {
  return (
    <div className="flex items-center space-x-2">
      <label htmlFor={label}>{label}</label>
      <input
        className="w-16 border border-gray-300 rounded-md text-white bg-gray-800"
        id={label}
        type="number"
        value={value}
        step={0.01}
        min={0}
        onChange={(e) => {
          onChange?.(e.target.valueAsNumber);
        }}
      />
    </div>
  );
};

const ParameterController = () => {
  const { promise } = useControlData();
  const controlData = use(promise);
  const [data, setData] = useState<ControlData>(
    controlData ?? defaultControlData
  );

  const handleSubmit = useCallback(async () => {
    await updateControlData(data);
  }, [data]);

  return (
    <div className="space-x-5 flex">
      <InputControlParam
        label="ki"
        value={data.ki}
        onChange={(ki) => {
          setData({ ...data, ki: ki });
        }}
      />
      <InputControlParam
        label="kd"
        value={data.kd}
        onChange={(kd) => {
          setData({ ...data, kd: kd });
        }}
      />
      <InputControlParam
        label="kp"
        value={data.kp}
        onChange={(kp) => {
          setData({ ...data, kp: kp });
        }}
      />
      <InputControlParam
        label="target"
        value={data.target}
        onChange={(target) => {
          setData({ ...data, target: target });
        }}
      />
      <button
        className="px-4 py-2 bg-blue-500 text-white rounded-md"
        onClick={handleSubmit}
      >
        Submit
      </button>
    </div>
  );
};

export default ParameterController;
