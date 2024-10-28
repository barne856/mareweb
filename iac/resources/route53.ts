import { TerraformStack, TerraformVariable } from "cdktf";
import { Route53Record } from "@cdktf/provider-aws/lib/route53-record";
import { DataAwsRoute53Zone } from "@cdktf/provider-aws/lib/data-aws-route53-zone";
import { CloudfrontDistribution } from "@cdktf/provider-aws/lib/cloudfront-distribution";

export function configureRoute53(
  stack: TerraformStack,
  distribution: CloudfrontDistribution,
  domainName: TerraformVariable,
): void {
  // get the route53 zone
  const zone = new DataAwsRoute53Zone(stack, "website-route53-zone", {
    name: domainName.value,
    privateZone: false,
  });

  // create the route53 record
  new Route53Record(stack, "website-route53-record", {
    dependsOn: [distribution, zone],
    name: domainName.value,
    zoneId: zone.zoneId,
    type: "A",
    alias: {
      name: distribution.domainName,
      zoneId: distribution.hostedZoneId,
      evaluateTargetHealth: false,
    },
  });
}
