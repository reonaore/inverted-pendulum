"use client";
import { updateControlData } from "@/actions/actions";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { use, useCallback, useState } from "react";
import { ControlData, useControlData } from "../hooks/useControlData";

import {
  Dialog,
  DialogClose,
  DialogContent,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";
import { Settings } from "lucide-react";
import { toast } from "sonner";

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
    <div className="space-y-2">
      <Label htmlFor={label}>{label}</Label>
      <Input
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
    const promise = updateControlData(data);
    toast.promise(promise, {
      loading: "Loading...",
      success: () => {
        return "Parameters updated";
      },
      error: "Failed to update parameters",
    });
  }, [data]);

  return (
    <Dialog>
      <DialogTrigger asChild>
        <Button variant="secondary">
          <Settings /> Change Parameters
        </Button>
      </DialogTrigger>
      <DialogContent>
        <DialogHeader>
          <DialogTitle>Change Parameters</DialogTitle>
        </DialogHeader>
        <div className="space-y-4">
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
        </div>
        <DialogFooter className="flex justify-end mt-10">
          <DialogClose asChild>
            <Button onClick={handleSubmit}>Submit</Button>
          </DialogClose>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
};

export default ParameterController;
